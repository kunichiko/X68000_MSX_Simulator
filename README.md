# About MS.X

This is MSX Simulator working on the X68000 series computers. I made it when I was teenager and it is just my study, so it is not completed.

But I decided to make it public for my memorial.

# How to use (in Japanese)

実行にはMC68030以上が必要です。Phantom Xをお使いの場合は、

```
> pxctl -e 3
→その後物理リセット
```

などでCPUを030仕様に変更してください。(ワード境界を跨ぐアクセスなどをしているためですが、将来修正するかもしれません)

実行は exe フォルダの下で行います。exe フォルダ以下にはCBIOSのROMファイルがおいてありますので、そのまま起動することができます(まだ画面が崩れたりしますが)。一方、CBIOSにはBASICが入っていないので、なんらかのゲームのROMイメージなどを使用しないと何もできません。

ROM起動のゲームを起動したい場合は、実機などでROMを吸い出し、以下のようにして起動してください（※現在のところまだメガロム二は対応しておらず、16Kバイト、もしくは32Kバイトのものにしか対応していません）

```
> ms.x -r GAME.ROM
```

このように実行すると GAME.ROM が 16Kバイトの場合はスロット1のページ1に、32Kバイトの場合はページ1と2にマップされた状態で起動します。

## 起動後の操作

* HELPキー
	* テキストプレーンに表示されているデバッグメッセージの表示/非表示を切り替えます
* 登録キー
	* MS.X を終了します
* 記号キー
	* デバッグ情報を出力します

## 制限

いっぱいあります。

* 遅い
	* 画面の書き換えをまだチューニングしていないので、画面書き換えの多いゲームは動作が重くなる傾向があります。後々改善予定です。
* 速い
	* 逆に、画面の書き換えが少ないゲームは、実機よりも速く動作することがあります
	* CPUサイクルをちゃんと数えるようにして、速度調整を入れる計画はありますが、まだ実装していません
* 落ちる
	* 不意にクラッシュしたり止まったりすることがあります
	* アプリがクラッシュすると、割り込み処理が登録されたままになり、その後の動作が不安定になることが多いです。そういった場合は X68000を再起動してください
* 再現性が低い
	* まだまだですね……
* FDDが使えない
	* まだ実装していません
* サウンドがおかしい
	* まだ適当です

## BIOSの差し替え

C-BIOSではなく、実機のROMを使いたい場合は、自分でダンプするなどしてご用意ください。

* MAINROM.ROM - メインROM 32KB
* SUBROM.ROM - サブROM 16KB

の２つをご用意ください。このファイルがある場合は、C-BIOSではなくこちらを優先して利用しますので、BASICを試すことができます。
BASICを使わず、ゲームなどを起動するだけであれば C-BIOSのままで問題ありません。

# Architecture (in Japanese)

## ゴール

完全なエミュレーションは不可能と考えているため、MSXの簡単なゲームやプログラムの動作確認ができる程度を目指しています。
MSXのコアな機能をフル活用したメガデモなどが動作することは目標としていません。

## CPUエミュレーション

自作の Z80 (R800) エミュレーターを使っています。フルアセンブラで書かれています。

基本的にはループをぐるぐる回してZ80の命令を逐一解釈して実行しているだけですが、Z80の割り込み処理や、X68000側のキー入力の対応など、定期的にループ処理を中断できるようになっています。
ループ処理の中断は cpu_yield かemu_yield に0以外をセットすることで行われます。現在はX68000側の垂直同期割り込みのタイミングで 0以外をセットしています。

エミュレーションループが中断すると、cpu_yieldとemu_yieldのそれぞれを0になるまでカウントダウンします。1から0になったタイミングで以下を実行します。

* cpu_yieldが0になったタイミング
	* EI状態でかつVDPの割り込みリクエストが立っていればZ80の割り込み処理を行います
	* ただ、エミュレーション速度が遅いと、割り込み処理から抜けられなくなってしまうので、ある一定数以上の負荷がかかっている場合は割り込みをスキップするようにしています
* emu_yieldが0になったタイミング
	* C言語側のルーチンに制御を戻し、キーボード入力の処理などを実行します
	* X68000 のIOCS BITSNSを使ってキーマトリクスを読み取り、MSXのキーマトリクスに変換し、MSXのPPI 8255AのI/O (port #A9)の読み出しで、そのマトリクスの値を返却しています
	* また、記号、登録、HELPキーなどによる特殊処理も実行します

cpu_yieldと emu_yieldの数には差をつけてあり、emu_yieldの方に大きな数を入れています。これにより、Z80の割り込み処理が優先されるようになっています。

## VDPエミュレーション

MSXの画面を X68000の画面に描画するため、以下のような構成にしています。

* MSXのスプライト
	* X68000のスプライト機能を使って描画
* MSXのグラフィック/テキスト
	* X68000のグラフィック機能を使って描画
* MSXのバックドロップ(裏に透けて見える色)
	* X68000のBGプレーンを使って描画

MSXの解像度はインターレースモードを含めると 最大512x424なので、これを X68000の 512x512の画面上に描画します。また、MSXの水平同期周波数は15kHzですが、X68000では31kHzの画面を出力しています。

MSXの画面モードの多くは15kHz、横256ドットなので、X68000の画面も15kHz、256ドットモードにした方がより実機に近づき、パフォーマンス上も有利かもしれませんがですが、今のところは31kHz、512ドットモードで描画することにしています。

これは、以下のような理由でそうしています。

* X68000を 31kHz専用ディスプレイ(液晶など)で使っている人も多く 15kHzに切り替わると映らないケースも多い
* 512ドットモードの方が、db.xでデバッグしやすい（文字が大きくなりすぎない）
* MSXは横512ドットモードの時でもスプライトは常に横256ドットの解像度で表示される。そのため、MSXの解像度に合わせてX68000の画面モードを変化させると、スプライト描画の処理が複雑になる
* MSXは256x256ドットで球面になっているが、X68000は512x512ドットで球面になっているため、スクロールへの対応がしやすい
	* 正確にはMSX2までは縦スクロールしかないが

### 画面描画の制限
MSXのTEXTモードは、パターンジェネレータテーブルと、パターンネームテーブルでできているので、たとえば、画面全体に "A"という文字を敷き詰めた状態で、"A"のパターンを書き換えると、画面全体の"A"の表示が書き換わる。
これを正確にエミュレーションしようとすると、パターンの書き換え時に、画面全体を書き換える必要があり、X68000的には結構負荷が高い(最悪の場合、512x512の全てを書き換えないといけない)ので、現在それは実装していません(垂直帰線期間中に書き換えをやろうとしてフラグを作った形跡はある)。
X68000の場合BGを使えば似たようなことはできますが、PCGをスプライトの定義と共有しているので色々と制限があります。画面モードを 256x256モードにすれば、BGのパターンを 8x8で256個定義でき、かつスプライト用にも16x16の定義を128個確保するということもできるので、TEXTモードの実現は、こちらの方が楽かもしれません。

別解としては、テキストプレーンを使う方法もあります。テキストプレーンは1回の1ワード(16bit)書き込みで16ドットを描画できますが、これはMSXの8x8に相当するので都合が良いです。ただ、16色のうち2色を表示できるため、結局4プレーン全てを書き換える必要があり、4回の書き換えが必要になり、そんなに高速にはならないかも知れません。

ただ、SCREEN0系の画面(TEXT1モード、TEXT2モード)だとキャラクタが6x8ドットになり横に40(or 80)キャラクター並ぶので、この場合はBGを使って実現することはできません。
なので、いずれにせよSCREEN0系の画面の場合は、グラフィック面に描画するしかないと思われます。

## テキスト/グラフィック/スプライト(BG含む) の前後関係

MSXの画面は、背後から順に、

* バックドロップ面
	* 単色で塗りつぶされた領域 (これが見えない画面モードもある?はず)
* パターン面
	* テキストやグラフィックなど
	* 画面によってはカラーコード 0のところからバックドロップが見えたりする（んだったと記憶）
* スプライト面
	* スプライトが表示されているところ

という順に並んでいます。X68000の場合スプライトとBG面は同じグループに属しているので、MSXのスプライトを X68000のスプライトで実現すると、必然的にBGも手前に来ることになります。
gbdserverによるリモートデバッグもできるようになったので、デバッグ用にテキスト画面を使うことを諦めれば、以下のようにするのが最も効率がよさそうです。

* TEXT1,TEXT2 (SCREEN0系)
	* モード: 512x512, 16色
	* 割り当て(背後から順に)
		* グラフィック (描画面)
		* テキスト (マスク面)
* MULTI COLOR (SCREEN3)
	* モード: 512x512, 16色
	* 割り当て(背後から順に)
		* グラフィック (描画面)
		* TBD
		* テキスト (マスク面)
* GRAPHIC1 (SCREEN1)
	* TBD


## スプライトエミュレーション

スプライトは、X68000のスプライトを使って実現します。

MSXのスプライトの仕様は以下の通りです。

* サイズ
	* 8x8 or 16x16
	* 2倍に拡大するモードもある
* パターン
	* 1パターンにつき 1色
	* スプライトモード2では重ね合わせで多色化も可能
* 横に並べられるスプライト数
	* モード1(MSX1) 4個
	* モード2(MSX2) 8個
* 登録できるパターン数
	* 8x8換算で、256個
* 同時に表示できる数
	* 32個

一方、X68000のスプライトは以下の通りです。

* サイズ
	* 16x16
* パターン
	* 1パターンにつき 16色
* 横に並べられるスプライト数
	* 32個
* 登録できるパターン数
	* 16x16で、128個 (BGを諦めれば256個)
* 同時に表示できる数
	* 128個

ですが、X68000側の解像度は512x512なので、MSXのスプライトの1ドットは、X68000では2ドットになります。つまり以下のようになります。

* MSXの 8x8ドットモード、拡大なしの場合
	* 1スプライトを 16x16ドットで描画する
	* MSXは同時に32個表示できるが、X68000は16x16を128個表示できるので問題ない
	* 横に最大 32個並べることができてしまう
		* 同じスプライトを4つずつ重ねて表示することで、横8個の制限を再現することは可能
		* 同じスプライトを8個重ねれば横4個の制限も可能だが、8 x 32で 256個表示する必要があるので、スプライトダブラが必要になる
	* MSXは8x8の定義を256個作ることができるが、X68000のスプライトは16x16の定義を128個しか登録できない
* MSXの 16x16ドットモード、拡大なしの場合
	* 1スプライトを 16x16ドットを2x2個ずつ 4つ並べ、32x32ドットで描画する
	* MSXは同時に32個表示できるが、X68000は32x32を32個(16x16を128個)表示できるので問題ない
	* 横に最大 16個並べることができてしまう
		* 同時表示32個でギリギリなので、同じスプライトを重ねて表示することによる横4個 or 横8個の制限を再現することは難しい(スプライトダブラが必要)
	* MSXは16x16の定義を64個作ることができるが、X68000のスプライトは32x32の定義を32個しか登録できない
* MSXの 8x8ドットモード、拡大ありの場合
	* 1スプライトを 16x16ドットを2x2個ずつ 4つならべ、32x32ドットで描画する
	* MSXは同時に32個表示できるが、X68000は32x32を32個表示できるので問題ない
	* 横に最大 16個並べることができてしまう
		* 同時表示32個でギリギリなので、同じスプライトを重ねて表示することによる横4個 or 横8個の制限を再現することは難しい(スプライトダブラが必要)
	* MSXは8x8の定義を256個作ることができるが、X68000のスプライトは32x32の定義を32個しか登録できない
* MSXの 16x16ドットモード、拡大ありの場合
	* 1スプライトを 16x16ドットを4x4個ずつ 16個並べ、64x64ドットで描画する
	* MSXは同時に32個表示できるが、X68000は64x64を8個しか表示できないので、スプライトダブラが必要
	* 横に最大 8個並べることができてしまう
		* 同時表示数が足りてないので、同じスプライトを重ねて表示することによる横4個の制限を再現するにはスプライトダブラが必要
	* MSXは16x16の定義を64個作ることができるが、X68000のスプライトは64x64の定義を8個しか登録できない

以上のようになるため、少なくともMSXのスプライトの拡大モードは実現が難しそうです。もし将来拡大モードを実現する場合は、たとえばテキストプレーンに描画するなどの方法を考える必要があります。

そこで、まずは拡大なしの8x8ドットモード、16x16ドットモードのスプライトを実装することにしました。

その場合でも、PCGの定義数が足りないので、以下のようにして実現します。

* MSXは同時に32個しか表示できないので、X68000側は、その表示中の32個の定義だけをPCGに登録する
* X68000側はPCGの128個の定義と SSR(スプライトスクロールレジスタ)を一対一に対応させる
* MSXのスプライト番号と、X68000のPCG、SSR(スプライトスクロールレジスタ)の対比は以下のようにする
	* SP #0
		* PCG #0-#3
		* SSR #0-#3
	* SP #1
		* PCG #4-#7
		* SSR #4-#7
	* SP #31
		* PCG #124-#127
		* SSR #124-#127

こうすると、たとえばMSX側でスプライト番号0のパターンを nに変更した場合は、X68000側ではPCG #0-#3をn番のパターンで書き換えることで実現できます。MSX側で複数のスプライト番号に同じパターンnをセットした場合でも、X68000側はそれぞれのPCGにn番のパターンをセットすることになります(定義が冗長になるが、処理が簡単)。

## PSGエミュレーション

YM2151を使って似たような波形を作って鳴らします。
エンベロープの再現がちょっと大変なので、どうするか悩んでいます。割り込みで音量を調整してエンベロープを再現するしかないかもしれませんが、まずはそれっぽい音階の音が出ることまでを目標にしています。

## FDCエミュレーション

フロッピーディスクのエミュレーションもしたいところですが、おそらくBIOSをハックする必要があるので、すぐには難しいかもしれません。
MSXのC-BIOSのようなものをベースにすればできるかも？
