# CDA_Automata

圧縮文字列辞書をオートマトンで実現し、ダブル配列で表現した実験用ライブラリ．
「松本,神田,森田,泓田, ダブル配列オートマトンによる圧縮文字列辞書の実装, 情報処理学会IFAT研究会, 2018」の実装になります．
実装に用いたデータ構造については「松本,神田,森田,泓田, ダブル配列オートマトンの圧縮手法, DEIM, 2018」を御参照下さい．

現時点で研究用のため，ご利用は自己責任でお願い致します．

## 文字列辞書概要

文字列集合と，文字列に対応するIDを保存し双方向の検索を実現するデータ構造．
プログラム言語のデータ構造でいうと，C++の std::map<std::string, int>, Pythonの Dictionary{str: int} のような機能を実現するものです．
大規模なデータセットに対しても，実用的なメモリサイズと検索速度で辞書を実現できるようになります．
本ライブラリはデータセットの入力に対して静的辞書を構築し提供するため，構築後の辞書の更新には対応していません．

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
 - Python3: "data-sets/create-test-datasets.py" の実行に必要

 また，実験用スクリプトで以下のコマンドを使用します
 - gtime: "02-build-dicts.sh" での計測に必要（参考：https://apple.stackexchange.com/questions/193986/to-install-gnu-time-in-osx）
 - GNU parallel: "02-build-dicts.sh" 実行時の並列化実行に必要(https://www.gnu.org/software/parallel/)($brew install parallel　も可能)

 Optional
 - JDK: Dawid Weissによる実装を動作させるために必要（参考：https://eng-entrance.com/java-install-jdk-mac）

## 構成
 - *.sh: このライブラリで各辞書のベンチマークがとれるまでの処理を一通り記述したスクリプト．順番通り呼び出せばとりあえず結果が得られる．
 - array-fsa: 神田によるオートマトンの配列表現の実装（C++）
 - data-sets: ベンチマークに用いるデータセット群．ダウンロードはディレクトリ内のREADMEを参照
   - ciura-deorowicz/weiss:  「[paper-fsa-compression][pfc]」からの拝借物
   - kanda: 神田の収集したデータセット
 - software: 主に比較手法として用いるデータ構造
   - XOR圧縮を用いたダブル配列トライ: [XOR-Compressed Double-Array Trie][xcdat]
   - Path decomposition によるトライの圧縮表現: [Path-Decomposed tries][pdt]
   - Dawid Weissによる圧縮オートマトン表現の実装（Java）: [paper-fsa-compression][pfc]
   - darts-clone: ダブル配列ベースの高速辞書: [darts-clone][darts-clone]

[pfc]: https://github.com/dweiss/paper-fsa-compression
[xcdat]: https://github.com/kampersanda/xcdat
[pdt]: https://github.com/ot/path_decomposed_tries
[darts-clone]: https://github.com/s-yata/darts-clone

data-setsには「\*.dict」と「\*.1000000.rnd_dict」という二種類のデータセットが含まれており，前者が辞書を構築するときに用いるデータセットで，後者が検索時間を測るときに用いるベンチマーク用データセットです．
データセットはGitに含まれないため，/data-sets/README.md に従ってダウンロードしてください．
