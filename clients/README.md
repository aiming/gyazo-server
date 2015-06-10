# gyazo server


## Description

* Mac
   * http用とhttps用があります
   * Vagrant でテストするには、こちらのhttp用(http-for-test-Gyazo.app)を使ってください

* Win
   * https用のみです

## Usage

以下を自分たちのserver名に変更して下さい

* MacOSX/Gyazo.app/Contents/Resources/script
  - 51:HOST='example.com'

* Win/gyazowin+.ini
  - 2:upload_server=example.com
