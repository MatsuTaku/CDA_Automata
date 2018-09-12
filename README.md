# CDA_Automata

圧縮文字列辞書をオートマトンで実現し、ダブル配列をベースとしたデータ構造で表現した実験用ライブラリ．
開発中のため，ご利用は自己責任でお願い致します．
本ライブラリは「松本,神田,森田,泓田, ダブル配列オートマトンによる圧縮文字列辞書の実装, 情報処理学会IFAT研究会, 2018」の実装になります．
実装に用いたデータ構造については「松本,神田,森田,泓田, ダブル配列オートマトンの圧縮手法, DEIM, 2018」を参照して下さい．

## 導入
このライブラリはサブモジュールを用いているため，以下のコマンドでクローンして下さい．
```
git clone --recursive https://github.com/MatsuTaku/CDA_Automata.git
```
または，クローン後に以下のコマンドを実行して下さい．
```
git submodule update --init --recursive
```

## 開発環境
 環境はmacOSを想定しているため，プロジェクト内のスクリプトはMAC上でのコマンドで記述されています．
 このライブラリを動作させるには以下の環境が必要になります．
 - CMake: コンパイルに必要
 - Python3: 「data-sets/create-test-datasets.py」のために必要
 - gtime：「02-build-fsa-dicts.sh」で必要（参考：https://apple.stackexchange.com/questions/193986/to-install-gnu-time-in-osx）

 Optional
 - JDK: Dawid Weissによる実装を動作させるために必要（参考：https://eng-entrance.com/java-install-jdk-mac）

## 構成
 - *.sh：このライブラリで各辞書のベンチマークがとれるまでの処理を一通り記述したスクリプト．順番通り呼び出せばとりあえず結果が得られる．
 - array-fsa: 神田によるオートマトンの配列表現の実装（C++）
 - data-sets: ベンチマークに用いるデータセット群．ダウンロードはディレクトリ内のREADMEを参照
   - ciura-deorowicz/weiss:  「[paper-fsa-compression][pfc]」からの拝借物
   - kanda: 神田の収集したデータセット
 - software: 主に比較手法として用いるデータ構造
   - XOR圧縮を用いたダブル配列トライ: [XOR-Compressed Double-Array Trie][xcdat]
   - Path decomposition によるトライの圧縮表現: [Path-Decomposed tries][pdt]
   - Dawid Weissによる圧縮オートマトン表現の実装（Java）: [paper-fsa-compression][pfc]

[pfc]: https://github.com/dweiss/paper-fsa-compression
[xcdat]: https://github.com/kampersanda/xcdat
[pdt]: https://github.com/ot/path_decomposed_tries

data-setsには「\*.dict」と「\*.1000000.rnd_dict」という二種類のデータセットが含まれており，前者が辞書を構築するときに用いるデータセットで，後者が検索時間を測るときに用いるベンチマーク用データセットです．
