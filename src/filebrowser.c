#define _XOPEN_SOURCE 700
#define _GNU_SOURCE

#include <stdbool.h>
#include <stdio.h>
#include <gmodule.h>
#include <cairo.h>
#include <nkutils-xdg-theme.h>
#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>

#include "config.h"
#include "types.h"
#include "files.h"
#include "icons.h"
#include "keys.h"
#include "util.h"
#include "options.h"

G_MODULE_EXPORT Mode mode;

/**
 * If not in dmenu mode, opens the file at the given path with the given command.
 * If in dmenu mode, prints the absolute path to stdout.
 */
static void open_file(char *path, char *cmd, FileBrowserModePrivateData *pd);

// ================================================================================================================= //

static int file_browser_init ( Mode *sw )
{
    if ( mode_get_private_data ( sw ) == NULL ) {
        FileBrowserModePrivateData *pd = g_malloc0 ( sizeof ( * pd ) );
        mode_set_private_data ( sw, ( void * ) pd );

        pd->open_custom = false;
        pd->open_custom_index = -1;
        /* Other values are initialized by set_command_line_options( pd ). */

        if ( ! set_command_line_options ( pd ) ) {
            return false;
        }

        /* Set up icons if enabled. */
        FileBrowserIconData *id = &pd->icon_data;
        if ( id->show_icons ) {
            init_icons ( id );
        }

        /* Load the files. */
        load_files( &pd->file_data );
    }

    return true;
}

static void file_browser_destroy ( Mode *sw )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    mode_set_private_data ( sw, NULL );

    /* Free file list. */
    destroy_files ( &pd->file_data );

    /* Free icon themes and icons. */
    destroy_icons( &pd->icon_data );

    /* Free the rest. */
    g_free ( pd->cmd );
    g_free ( pd->show_hidden_symbol );
    g_free ( pd->hide_hidden_symbol );
    g_free ( pd->path_sep );

    /* Fill with zeros, just in case. */
    memset ( ( void * ) pd , 0, sizeof ( pd ) );

    g_free ( pd );
}

static unsigned int file_browser_get_num_entries ( const Mode *sw )
{
    const FileBrowserModePrivateData *pd = ( const FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    const FileBrowserFileData *fd = &pd->file_data;

    if ( pd->open_custom ) {
        return 1;
    } else {
        return fd->num_files;
    }
}

static ModeMode file_browser_result ( Mode *sw,  int mretv, char **input, unsigned int selected_line )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserKeyData *kd = &pd->key_data;

    ModeMode retv = RELOAD_DIALOG;
    FBKey key = get_key_for_rofi_mretv ( mretv );

    /* Handle open-custom prompt. */
    if ( pd->open_custom ) {
        if ( mretv & MENU_OK || key == kd->open_custom_key || key == kd->open_multi_key ) {
            char *cmd = ( *input != NULL && strlen ( *input ) == 0 ) ? pd->cmd : *input;
            char *path = fd->files[pd->open_custom_index].path;
            open_file ( path, cmd, pd );
            pd->open_custom = false;
            pd->open_custom_index = -1;
            if ( key != kd->open_multi_key ) {
                retv = MODE_EXIT;
            } else {
                retv = RESET_DIALOG;
            }
        } else if ( mretv & MENU_CANCEL ) {
            pd->open_custom = false;
            pd->open_custom_index = -1;
            retv = RESET_DIALOG;
        }

    /* Handle open-custom key press. */
    } else if ( key == kd->open_custom_key && selected_line != -1 ) {
        pd->open_custom = true;
        pd->open_custom_index = selected_line;
        retv = RESET_DIALOG;

    /* Handle return or open-multi. */
    } else if ( key == kd->open_multi_key || mretv & MENU_OK ) {
        FBFile* entry = &fd->files[selected_line];
        switch ( entry->type ) {
        case UP:
        case DIRECTORY:
            if ( key == kd->open_multi_key ) {
                open_file ( entry->path, pd->cmd, pd );
            } else {
                change_dir ( entry->path, fd );
                retv = RESET_DIALOG;
            }
            break;
        case RFILE:
        case INACCESSIBLE:
            open_file ( entry->path, pd->cmd, pd );
            if ( key != kd->open_multi_key ) {
                retv = MODE_EXIT;
            }
            break;
        }

    /* Handle custom input or Control+Return. */
    } else if ( mretv & MENU_CUSTOM_INPUT ) {
        if ( strlen ( *input ) > 0 ) {
            char *expanded_input = rofi_expand_path ( *input );
            char *file = g_filename_from_utf8 ( expanded_input, -1, NULL, NULL, NULL );
            g_free ( expanded_input );

            char *abs_path = get_existing_abs_path ( file, fd->current_dir );
            g_free ( file );

            if ( abs_path == NULL ) {
                retv = RELOAD_DIALOG;
            } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_DIR ) ){
                change_dir ( abs_path, fd );
                retv = RESET_DIALOG;
            } else if ( g_file_test ( abs_path, G_FILE_TEST_IS_REGULAR ) ) {
                open_file(abs_path, pd->cmd, pd);
                retv = MODE_EXIT;
            }

            g_free ( abs_path );
        }

    /* Toggle hidden files with toggle_hidden_key. */
    } else if ( key == kd->toggle_hidden_key ) {
        fd->show_hidden = ! fd->show_hidden;
        load_files ( fd );
        retv = RELOAD_DIALOG;

    /* Enable hidden files with Shift+Right. */
    } else if ( kd->use_mode_keys && ( mretv & MENU_NEXT ) && !fd->show_hidden ) {
        fd->show_hidden = true;
        load_files ( fd );
        retv = RELOAD_DIALOG;

    /* Disable hidden files with Shift+Left. */
    } else if ( kd->use_mode_keys && ( mretv & MENU_PREVIOUS ) && fd->show_hidden ) {
        fd->show_hidden = false;
        load_files ( fd );
        retv = RELOAD_DIALOG;

    /* Default actions */
    } else if ( mretv & MENU_CANCEL ) {
        retv = MODE_EXIT;
    } else if ( mretv & MENU_NEXT ) {
        retv = NEXT_DIALOG;
    } else if ( mretv & MENU_PREVIOUS ) {
        retv = PREVIOUS_DIALOG;
    } else if ( mretv & MENU_QUICK_SWITCH ) {
        retv = ( mretv & MENU_LOWER_MASK );
    }

    return retv;
}

static int file_browser_token_match ( const Mode *sw, rofi_int_matcher **tokens, unsigned int index )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;

    if ( pd->open_custom ) {
        return true;
    } else {
        return helper_token_match ( tokens, fd->files[index].name );
    }
}

static char *file_browser_get_display_value ( const Mode *sw, unsigned int selected_line, G_GNUC_UNUSED int *state,
        G_GNUC_UNUSED GList **attr_list, int get_entry )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;

    if ( !get_entry ) return NULL;

    int index;
    if ( pd->open_custom ) {
        index = pd->open_custom_index;
    } else {
        index = selected_line;
    }

    return rofi_force_utf8 ( fd->files[index].name, strlen( fd->files[index].name ) );
}

static cairo_surface_t *file_browser_get_icon ( const Mode *sw, unsigned int selected_line, int height )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;

    if ( ! id->show_icons ) {
        return NULL;
    }

    int index;
    if ( pd->open_custom ) {
        index = pd->open_custom_index;
    } else {
        index = selected_line;
    }

    if ( fd->files[index].icon == NULL ) {
        fd->files[index].icon = get_icon_for_file( &fd->files[index], height, &pd->icon_data );
    }

    return fd->files[index].icon;
}

static char *file_browser_get_message ( const Mode *sw )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;

    if ( pd->open_custom ) {
        char* file_name = fd->files[pd->open_custom_index].name;
        char* message = g_strdup_printf ( OPEN_CUSTOM_MESSAGE_FORMAT, file_name );
        return message;

    } else if ( pd->show_status ) {
        char** split = g_strsplit ( fd->current_dir, G_DIR_SEPARATOR_S, -1 );
        char* join = g_strjoinv ( pd->path_sep, split );
        char* message = g_strconcat ( fd->show_hidden ? pd->show_hidden_symbol : pd->hide_hidden_symbol, join, NULL );
        char* utf8_message = rofi_force_utf8( message, strlen ( message ) );

        g_strfreev ( split );
        g_free ( join );
        g_free ( message );

        return utf8_message;

    } else {
        return NULL;
    }
}

// ================================================================================================================= //

static void open_file ( char *path, char *cmd, FileBrowserModePrivateData *pd )
{
    if ( pd->dmenu ) {
        printf("%s\n", path);

    } else {
        /* Escape the file path. */
        char **split = g_strsplit ( path, "\"", -1 );
        path = g_strjoinv ( "\\\"", split );
        g_strfreev ( split );

        /* Construct the command. */
        char* complete_cmd = NULL;
        if ( g_strrstr ( cmd, "%s" ) != NULL ) {
            complete_cmd = g_strdup_printf ( cmd, path );
        } else {
            complete_cmd = g_strconcat ( cmd, " \"", path, "\"", NULL );
        }

        helper_execute_command ( pd->file_data.current_dir, complete_cmd, false, NULL );

        g_free ( complete_cmd );
    }
}

// ================================================================================================================= //

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "file-browser",
    .cfg_name_key       = "display-file-browser",
    ._init              = file_browser_init,
    ._get_num_entries   = file_browser_get_num_entries,
    ._result            = file_browser_result,
    ._destroy           = file_browser_destroy,
    ._token_match       = file_browser_token_match,
    ._get_display_value = file_browser_get_display_value,
    ._get_icon          = file_browser_get_icon,
    ._get_message       = file_browser_get_message,

    ._get_completion    = NULL,
    ._preprocess_input  = NULL,
    .private_data       = NULL,
    .free               = NULL,
};
