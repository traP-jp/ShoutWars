# ShoutWars

**© 2024 traP Community**  

traP ワンマンソン 2024 レジェンドクリエイターズのゲーム

サーバー: [traP-jp/ShoutWars-server](https://github.com/traP-jp/ShoutWars-server)  
ホームページ: [traP-jp/ShoutWars-web](https://github.com/traP-jp/ShoutWars-web)

## ライセンス

traP Community が制作したアセットには以下のライセンスが適用されます。

- ソースコード: [MIT License](LICENSE)  
- 画像素材: [CC BY-NC-ND 4.0](https://creativecommons.org/licenses/by-nc-nd/4.0/deed.ja)  
- 音声素材: [CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/deed.ja)

第三者が権利を持つ素材は [`CREDITS.ini`](CREDITS.ini) に「収録先」付きで列挙してあり、それぞれのライセンスに従います。

## 開発メンバー

### レジェンドクリエイターズ (２班)

- [**U.N.ABC** (traP)](https://trap.jp/author/U-N-ABC) 主にゲームロジックプログラミング担当
- [**りすりす/TwoSquirrels** (traP)](https://trap.jp/author/TwoSquirrels) 主に通信・音声処理プログラミング担当
- [**いのちだいにに** (traP)](https://trap.jp/author/inochidainini) 主にサウンド・ゲームデザイン担当
- [**nemlos** (traP)](https://trap.jp/author/nemlos5) グラフィック担当

### ２班メンター

- [**けんけん** (traP)](https://trap.jp/author/kenken)
- [**門見** (traP)](https://trap.jp/author/Cd_48)

<!--
### 貢献者
-->

## 使用技術

- 言語: **C++23**
- ゲームフレームワーク: [**Siv3D** v0.6.16](https://github.com/Siv3D/OpenSiv3D/tree/v0.6.16)

## 開発にあたって

- Visual Studio 2026 と Siv3D v0.6.16 が必要です。
- 素材は Git LFS で管理しているため、クローン前に `git lfs install` を実行する必要があります。
- 通信先のサーバーは `App/config.json` に `{ "server": { "url": "https://shoutwars.trap.games/develop/api", "password": "..." } }` のように書くと切り替えられます。書かなければ本番サーバー (`https://shoutwars.trap.games/api`) に繋ぎます。`server` に `"syncLogDirectory": "logs"` を足すと、同期ごとの通信時間を `App/logs/` に CSV で書き出します。
- 展示では `App/config.json` に `"exhibition": true` を足すと、フルスクリーンで起動し、起動のたびに前の人のキャリブレーション (入力感度以外) を消し、画面の左下に ESC で終了できることを表示します。
- ゲームに再配布できない素材が含まれていたため、v0.2.1 以前のコミット履歴から画像・音声素材が全て削除されています。そのため、それ以前のビルドを再現することはできないことに注意してください。
