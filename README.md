# array-fsa

このライブラリは神田が実験用に作ったもので，オートマトンの配列表現の実装に加えて，「Smaller representation of finite state automata」における*fsa5*と*cfsa2*との比較もおこなえる結構気の利いたライブラリです．

## 開発環境

本ライブラリは環境としてmacOSを想定しています．なので同梱されているシェルスクリプトはMac上でのコマンドを用いて記述しています．ソースコード自体はUnixであれば動くはずです（たぶん）．Windowsは知りません．ちなみに自分はこのライブラリをmacOS Sierra上で作成しました．

このライブラリを動作させるためには，以下の環境が必要になります．

- CMake：神田による実装をコンパイルするのに必要
- JDK：Dawid Weissによる実装を動作させるために必要（参考：https://eng-entrance.com/java-install-jdk-mac）
- Python3：「data-sets/create-test-datasets.py」のために必要
- gtime：「02-build-fsa-dicts.sh」で必要（参考：https://apple.stackexchange.com/questions/193986/to-install-gnu-time-in-osx）


## 構成

- *.sh：このライブラリで各オートマトンのベンチマークがとれるまでの処理を一通り記述したスクリプト．順番通り呼び出せばとりあえず結果が得られる．
- array-fsa：神田によるオートマトンの配列表現の実装（C++）
- data-sets：ベンチマークに用いるデータセット群
- software：Dawid Weissによる圧縮オートマトン表現の実装（Java）

data-setsとsoftwareは，ライブラリ「paper-fsa-compression」（https://github.com/dweiss/paper-fsa-compression）からの拝借物です．data-setsには「\*.dict」と「\*.1000000.rnd_dict」という二種類のデータセットが含まれており，前者が辞書を構築するときに用いるデータセットで，後者が検索時間を測るときに用いるベンチマーク用データセットです．

## おわりに

もっといろいろ説明すべきことはありますが，体力切れなのでREADMEはこれくらいです．シェルスクリプトの内容と神田のソースコードを追えば，このライブラリの具体的な構成はわかるはずです．わからないことがあれば随時聞いて頂ければと思います．

このライブラリを理解するためには，少なくとも以下の論文は読んでおく必要があるでしょう．

- Jan Daciuk, "Experiments with Automata Compression," *CIAA*, 2001.
- Jan Daciuk and Dawid Weiss, "Smaller representation of finite state automata," *Theoretical Computer Science*, 2012.
- 前田敦司 and 水島宏太, "オートマトンの圧縮配列表現と言語処理系への応用," *プログラミングシンポジウム*, 2008.

その他に，オートマトンやダブル配列，C++に関する基礎知識が無論必要になりますが，その辺りは並行して習得していけばよいでしょう．