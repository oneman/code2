# Maintainer: David Richards <rawdod@gmail.com>
pkgname=1amcode
pkgver=1
pkgrel=1
pkgdesc="HyperIntegrated MetaSystem"
url="https://github.com/oneman/code2"
arch=('any')
license=('none')
depends=('gmp' 'cairo' 'pipewire')

package() {
  make
  install -Dm755 ./program.exe "${pkgdir}/usr/bin/k"
}
