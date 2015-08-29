# Twitter Plugin for Unreal Engine 4

Unreal Engine 4向けのTwitterプラグインです。
今は、OAuth認証とtweetのみをサポートしています。

現在アルファ版で、今後は機能追加と仕様変更があります。

## ビルドと実行

リポジトリには、VisualStudioのソリューションファイル(.sln)ファイルが含まれていません。
git cloneした後、.uprojectファイルを右クリックして「Generate Visual Studio project files」を実行した後、
生成されたソリューションファイル(.slnファイル)をVisualStudioで実行してください。

## サンプル

プラグインを利用するサンプルはプロジェクト内のTwitterExampleファイル（レベル）です。

## License

Apache License, Version 2.0とします。詳細はLicense.txtを参照してください。

本ライブラリでは一部改変したliboauthcpp(https://github.com/sirikata/liboauthcpp)とpicojson(https://github.com/kazuho/picojson)を利用しています。


