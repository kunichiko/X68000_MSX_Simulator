# MS.X におけるFDDエミュレーションのアーキテクチャ

一般的に、フロッピーディスク装置は、以下のようなモジュールで構成されているので、MS.Xでもソフトウェア的に同じように昨日単位でモジュールを分割するように設計します。

* フロッピーディスクメディア
	* 磁性体の上に、ある密度でデータを記録するもの
	* メディアのケースにはタイトルなど、内容を識別する情報が書かれている
	* ライトプロテクトスイッチがついている
* フロッピーディスクメディアを挿入するスロット
	* イジェクトボタンがあり、メディアを取り出したり、挿入したりすることがでくる
	* ライトプロテクトされているかどうかを検出するセンサーがある
* フロッピーディスクドライブ
	* メディアを回転させる、止める
	* ヘッドをシークさせる
	* データ読み書きする
* フロッピーディスクコントローラー
	* ホスト(Z80)からの指示を受け、ドライブを制御する
	* 読み込んだデータのデコードや、CRCの計算などを行う
* DISK BIOS
	* フロッピーディスクコントローラーをMSX-DOSから操作するためのAPIを提供するROM

このような構造にしておくと、HDDへの拡張や、ホストOS側のファイルシステムを仮想ディスクとして扱う機能(DirAsDsk)への対応などもやりやすくなります。

例えば、openMSXの実装を見ると、上記のモジュールを以下のようなクラス/インターフェースで抽象化しているようです。雰囲気を理解しやすくするために、実装されている主なメソッドも列挙しておきます。

* フロッピーディスクメディア
	* SectorAccessibleDisk クラス
		* readSector()
		* writeSector()
		* forceWriteProtect()
		* applyPatch()
		* getSha1Sum()
	* 継承: Diskサブクラス
		* getName()
		* 継承: SectorBasedDisk
			* 継承: DSKDiskImage
				* .DSK 形式のイメージファイルを読み込むクラス
		* 継承: DMKDiskImage
			* .DMK 形式のイメージファイルを読み込むクラス
* 挿入スロット
	* DiskContainerクラス
		* getSectorAccessibleDisk()
		* getContainerName()
		* diskChanged()
		* insertDisk(ファイル名)
		* isRomDisk()
	* 継承: DiskChangerクラス
		* getDriveName()
		* getDiskName()
		* peekDiskChanged()
		* forceDiskChange()
		* getDisk()
* フロッピーディスクドライブ
	* DiskDrive 抽象クラス
		* isDiskInserted()
		* isWriteProtected()
		* isDoubleSided()
		* isTrack00()
		* setSide()
		* getSide()
		* step()
		* setMotor()
		* getMotor()
		* indexPulse()
		* writeTackByte()
		* readTrackByte()
		* getNextSector()
		* flushTrack()
		* diskChanged()
	* 実装(継承): DummyDriveクラス
	* 実装(継承): RealDriveクラス
	* 実装(継承): DriveMultiplexerクラス
		* WD2793BasedFDCクラスで複数ドライブを実現するためのもの？
* フロッピーディスクコントローラー
	* WD2793クラス
		* コンストラクタで DiskDriveが渡される
	* TC8566AFクラス
		* コンストラクタで DiskDriveが渡される
* DISK BIOS
	* MSXFDCクラス (MSXDeviceクラスを継承)
		* writeMem()
		* readMem()
		* MSXFDCクラスは4つのDiskDrive実装を持つが、コンフィグファイルを元に構築され、n個は RealDrive、4-n個は DummyDriveとして構築されるようになっている
	* 継承: WD2793BasedFDCクラス
		* DriveMultiplexerと協調動作して複数ドライブ対応を行なっているようだ
		* 継承: ToshibaFDCクラス
		* 継承: CanonFDCクラス
		* 継承: NationalFDCクラス
		* 継承: YamahaFDCクラス
	* 継承: TurboRFDCクラス
		* TC8566AFが接続される
		* TurboRFDCが実質的に TC8566AFBasedFDCとして動いているようだ
			* システム構成で "TC8566AF" が指定されている場合、TurboRFDCをアタッチしているのがわかる
				* https://github.com/openMSX/openMSX/blob/4ad949c8d7c80449a17b367f06bb0672f2c488df/src/DeviceFactory.cc#L237

読み解くといろいろと歴史的な経緯もありそうな雰囲気で、ものすごく綺麗に整理されているわけではないですが、かなり参考になります。

MS.Xではまず、TC8566AFベースのFDCをエミュレーションすることを目標としますが、将来的には他のFDCもエミュレーションできるようにしておきたいと考えており、以下のような構成にしようと考えています。

* ディスクメディア
	* ms_disk_media型
		* 継承: ms_disk_media_sectorbase型
			* 継承: ms_disk_media_dskformat型
		* 継承: ms_disk_media_rawtrackbase型
			* 継承: ms_disk_media_dmkformat型
* 挿入スロット
	* ms_disk_container型
		* 継承: ms_disck_container_floppy型
* ディスクドライブ
	* ms_disk_drive型
		* 継承: ms_disk_drive_floppy型
		* 継承: ms_disk_drive_none型
* ディスクコントローラー
	* ms_disk_controller型
		* 継承: ms_disk_controller_tc8566af型
* DISK BIOS
	* ms_disk_bios型 (ms_memmap型を継承)
		* 継承: ms_disk_bios_tc8566af型

将来的には、X68000実機に接続された 3.5インチ 2DD対応ドライブを直接使えるようにもしたいと考えています。TC8566AFへのコマンドをそのまま X68000の FDCにバイパスする方法もありうるかもしれませんが、タイミングがシビアなので、おそらくやるとしたら 9SCDRV.Xを使い、そちらのAPIを経由してアクセスすることになると考えています。

その場合は、上記 ms_disk_drive型の継承クラスとして、ms_disk_drive_9scdrv などを追加することになります。DISK BIOSはTC8566AF用のものを使い、TC8566AFからドライブに対するコマンドの発行を、9SCDRV.Xに渡すことで実現できるのではと思っています。

# ディスクイメージファイル

openMSXでは、XSADiskImage、DMKDiskImage、DSKDiskImageの3種類のディスクイメージファイルをサポートしています。これらのフォーマットは、それぞれ以下のような特徴があります。

* XSADiskImage
	* 圧縮されたSDKファイル
* DMKDiskImage
	* [opeMSXの詳細説明](https://github.com/openMSX/openMSX/blob/master/doc/DMK-Format-Details.txt) によると、元々は TRS-80 というコンピュータ向けに作られたフォーマットだそうです。
	* MFMエンコードなど、より生の情報に近い情報を格納することが可能です
* DSKDiskImage
	* 2D/2DDの単純なセクター情報を格納したフォーマット
	* ギャップの情報など、プロテクトに関係する情報は含まれていないため、プロテクトの再現などはできない

将来的にはDMKもサポートしたいですが、まずはシンプルな DSK 方式をサポートしようと思います。



