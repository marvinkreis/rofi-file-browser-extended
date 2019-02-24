pkgname=rofi-file-browser-extended
pkgver=0.1.0
pkgrel=1
pkgdesc="Plugin to use rofi as a file browser"
url="https://github.com/marvinkreis/${pkgname}"
arch=("i686" "x86_64")
license=("MIT")
depends=("rofi" "json-c")
makedepends=("git")

source=("git://github.com/marvinkreis/${pkgname}.git")
md5sums=("SKIP")

prepare() {
    cd "${srcdir}/${pkgname}"
    git submodule init
    git submodule update
}

build() {
    cd "${srcdir}/${pkgname}"
    autoreconf --install
    ./configure
    make
}

package() {
    cd "${srcdir}/${pkgname}"
    make DESTDIR="$pkgdir" PREFIX=/usr install
}
