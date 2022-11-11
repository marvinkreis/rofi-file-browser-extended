#include <stdbool.h>
#include <stdio.h>
#include <gmodule.h>
#include <cairo.h>
#include <rofi/mode.h>
#include <rofi/helper.h>
#include <rofi/mode-private.h>
#include <rofi/rofi-icon-fetcher.h>

#include "defaults.h"
#include "types.h"
#include "files.h"
#include "icons.h"
#include "keys.h"
#include "util.h"
#include "cmds.h"
#include "options.h"

G_MODULE_EXPORT Mode mode;

/**
 * If not in stdout mode, opens the file at the given path with the given command.
 * If in stdout mode, prints the absolute path to stdout.
 * If fbfile is given, uses the path of fbfile.
 * If fbfile is NULL, uses path.
 */
static void open_file ( FBFile *fbfile, char *path, char *cmd, FileBrowserModePrivateData *pd );

// ================================================================================================================= //

static int file_browser_init ( Mode *sw )
{
    if ( mode_get_private_data ( sw ) == NULL ) {
        FileBrowserModePrivateData *pd = g_malloc0 ( sizeof ( * pd ) );
        mode_set_private_data ( sw, ( void * ) pd );

        pd->open_custom = false;
        pd->open_custom_index = -1;
        /* Other values are initialized by set_options ( pd ). */

        if ( ! set_options ( pd ) ) {
            return false;
        }

        /* Load the files. */
        FileBrowserFileData *fd = &pd->file_data;
        if ( pd->stdin_mode ) {
            load_files_from_stdin ( fd );
        } else {
            load_files ( fd );
        }
    }

    return true;
}

static void file_browser_destroy ( Mode *sw )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );

    if ( pd == NULL ) {
        return;
    }

    mode_set_private_data ( sw, NULL );

    /* Free file list. */
    destroy_files ( &pd->file_data );

    /* Free icon themes and icons. */
    destroy_icon_data( &pd->icon_data );

    /* Free open-custom commands. */
    destroy_cmds ( pd );

    /* Free config-file options. */
    destroy_options ( pd );

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
        if ( pd->show_cmds ) {
            return pd->num_cmds;
        } else {
            return 1;
        }
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
        if ( mretv & MENU_OK || mretv & MENU_CUSTOM_INPUT || key == kd->open_custom_key || key == kd->open_multi_key ) {
            char* cmd;
            if ( pd->show_cmds && selected_line != -1 ) {
                cmd = pd->cmds[selected_line].cmd;
            } else {
                cmd = ( *input != NULL && strlen ( *input ) == 0 ) ? pd->cmd : *input;
            }
            open_file ( &fd->files[pd->open_custom_index], NULL, cmd, pd );
            pd->open_custom = false;
            pd->open_custom_index = -1;
            if ( key != kd->open_multi_key ) {
                write_resume_file ( pd );
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
        if ( pd->search_path_for_cmds ) {
            search_path_for_cmds ( pd );
            pd->search_path_for_cmds = false;
        }
        retv = RESET_DIALOG;

    /* Handle return or open-multi. */
    } else if ( ( mretv & MENU_OK || key == kd->open_multi_key ) && selected_line != -1 ) {
        FBFile* entry = &fd->files[selected_line];
        switch ( entry->type ) {
        case UP:
        case DIRECTORY:
        directory:
            if ( pd->no_descend || key == kd->open_multi_key ) {
                open_file ( entry, NULL, pd->cmd, pd );
                if ( key != kd->open_multi_key ) {
                    write_resume_file ( pd );
                    retv = MODE_EXIT;
                }
            } else {
                change_dir ( entry->path, fd );
                load_files ( fd );
                retv = RESET_DIALOG;
            }
            break;
        case RFILE:
        case INACCESSIBLE:
        file:
            open_file ( entry, NULL, pd->cmd, pd );
            if ( key != kd->open_multi_key ) {
                write_resume_file ( pd );
                retv = MODE_EXIT;
            }
            break;
        case UNKNOWN:
            if ( g_file_test ( entry->path, G_FILE_TEST_IS_DIR ) ) {
                goto directory;
            } else {
                goto file;
            }
        }

    /* Handle custom input or Control+Return. */
    } else if ( mretv & MENU_CUSTOM_INPUT ) {
        if ( strlen ( *input ) > 0 ) {
            char *expanded_input = rofi_expand_path ( *input );
            char *abs_path = get_canonical_abs_path ( expanded_input, fd->current_dir );
            g_free ( expanded_input );

            if ( ! g_file_test ( abs_path, G_FILE_TEST_EXISTS ) ) {
                retv = RELOAD_DIALOG;
            } else if ( ! pd->no_descend && g_file_test ( abs_path, G_FILE_TEST_IS_DIR ) ) {
                change_dir ( abs_path, fd );
                load_files ( fd );
                retv = RESET_DIALOG;
            } else {
                open_file ( NULL, abs_path, pd->cmd, pd );
                write_resume_file ( pd );
                retv = MODE_EXIT;
            }

            g_free ( abs_path );
        }

    /* Toggle hidden files with toggle_hidden_key. */
    } else if ( key == kd->toggle_hidden_key ) {
        fd->show_hidden = ! fd->show_hidden;
        load_files ( fd );
        retv = RELOAD_DIALOG;

    /* Default actions */
    } else if ( mretv & MENU_CANCEL ) {
        write_resume_file ( pd );
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
        if ( pd->show_cmds ) {
            FBCmd *fbcmd = &pd->cmds[index];
            return helper_token_match ( tokens, fbcmd->name != NULL ? fbcmd->name : fbcmd->cmd );
        } else {
            return true;
        }
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

    if ( pd->open_custom && pd->show_cmds ) {
        *state |= 8;
        FBCmd *fbcmd = &pd->cmds[selected_line];
        char* name = fbcmd->name != NULL ? fbcmd->name : fbcmd->cmd;
        return rofi_force_utf8 ( name, strlen ( name ) );
    } else {
        int index = pd->open_custom ? pd->open_custom_index : selected_line;
        FBFile *fbfile = &fd->files[index];
        return rofi_force_utf8 ( fbfile->name, strlen ( fbfile->name ) );
    }
}

static cairo_surface_t *file_browser_get_icon ( const Mode *sw, unsigned int selected_line, unsigned int height )
{
    FileBrowserModePrivateData *pd = ( FileBrowserModePrivateData * ) mode_get_private_data ( sw );
    FileBrowserFileData *fd = &pd->file_data;
    FileBrowserIconData *id = &pd->icon_data;

    if ( ! id->show_icons ) {
        return NULL;
    }

    if ( pd->open_custom && pd->show_cmds ) {
        FBCmd *fbcmd = &pd->cmds[selected_line];

        if ( fbcmd->icon_fetcher_request <= 0 ) {
            fbcmd->icon_fetcher_request = rofi_icon_fetcher_query ( fbcmd->icon_name, height );
        }
        return rofi_icon_fetcher_get ( fbcmd->icon_fetcher_request );

    } else {
        int index = pd->open_custom ? pd->open_custom_index : selected_line;
        FBFile *fbfile = & fd->files[index];

        if ( fbfile->icon_fetcher_requests == NULL ) {
            request_icons_for_file ( fbfile, height, id );
        }
        return fetch_icon_for_file ( fbfile );
    }
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

static void open_file ( FBFile* fbfile, char *path, char *cmd, FileBrowserModePrivateData *pd )
{
    char* current_dir = pd->file_data.current_dir;

    char* used_path;
    if ( fbfile != NULL ) {
        if ( pd->open_parent_as_self && fbfile->type == UP ) {
            used_path = current_dir;
        } else {
            used_path = fbfile->path;
        }
    } else {
        used_path = path;
    }

    char *canonical_path = get_canonical_abs_path ( used_path, current_dir );

    if ( pd->stdout_mode ) {
        printf( "%s\n", canonical_path );
        g_free ( canonical_path );

    } else {
        /* Escape the file path. */
        char **split = g_strsplit ( canonical_path, "\"", -1 );
        char *escaped_path = g_strjoinv ( "\\\"", split );

        /* Construct the command. */
        char* complete_cmd = NULL;
        if ( g_strrstr ( cmd, "%s" ) != NULL ) {
            complete_cmd = g_strdup_printf ( cmd, escaped_path );
        } else {
            complete_cmd = g_strconcat ( cmd, " \"", escaped_path, "\"", NULL );
        }

        helper_execute_command ( current_dir, complete_cmd, false, NULL );

        g_free ( canonical_path );
        g_strfreev ( split );
        g_free ( escaped_path );
        g_free ( complete_cmd );
    }
}

// ================================================================================================================= //

Mode mode =
{
    .abi_version        = ABI_VERSION,
    .name               = "file-browser-extended",
    .cfg_name_key       = "display-file-browser-extended",
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
