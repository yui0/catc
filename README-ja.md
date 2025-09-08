# catc 😺 - Tiny C Compiler using Lex & Yacc

🚀 **catc** は、Lex と Yacc を使って作られたシンプルで小さな C コンパイラです！
軽量で学習用途に最適、C言語の基本的なコンパイルを体験できます。✨

## 📖 概要

このプロジェクトは、Lex と Yacc を活用して C 言語のコードをアセンブリに変換する小さなコンパイラです。
シンプルな構造で、C言語の基本的な構文をサポート！
👉 コンパイラの仕組みを学びたい方や、軽量な C コンパイラを試したい方にピッタリです。

## 🛠️ 使い方

以下の手順で簡単に試せます！🎉

### 1. 依存パッケージのインストール
```bash
$ dnf install byacc flex
```

### 2. ビルド
```bash
$ make 🔨
```

### 3. コンパイルと実行
- テストコードをアセンブリに変換：
```bash
$ ./catc -S ./test/hello.c
	.section	.rodata
.LC0:
	.string	"Hello! tiny c world!!\n"
	.text
	.align	4
	.global	main
	.type	main,@function
main:
	pushl	%ebp
	movl	%esp,%ebp
	subl	$24,%esp
	movl	%ebx,-4(%ebp)
	lea	.LC0,%eax
	pushl	%eax
	call	printf
	movl	$0,%eax
	jmp	.L1
.L1:	movl	-4(%ebp),%ebx
	leave
	ret
```

- アセンブリをオブジェクトファイルに変換：
```bash
$ ./catc -S ./test/hello.c > ./test/hello.asm
$ as ./test/hello.asm -o ./test/hello.o
```

- 実行ファイルを作成して実行：
```bash
$ gcc ./test/hello.o -o ./test/hello
$ ./test/hello
Hello! tiny c world!!
```

- 直接実行も可能：
```bash
$ ./catc ./test/test01.c
$ ./test/test01
Hello world. 100+23=123
```

## 📚 ファイル構成
- `AST.c`, `AST.h`: 抽象構文木の定義と処理 🌳
- `clex.l`, `cparser.y`: Lex と Yacc のソースファイル 📝
- `x86_code_gen.c`: x86アセンブリコード生成 ⚡️
- `test/`: サンプルCコード 📂
- `Makefile`: ビルド用スクリプト 🔧

## 🔗 参考リンク
- [TakeWiki](http://www.pwv.co.jp/~take/TakeWiki/index.php?FrontPage) 📖
- [C Parser](http://www.syuhitu.org/other/cparse/cparse.html) 📚
- [Tiny C Note](http://www.hpcs.cs.tsukuba.ac.jp/~msato/lecture-note/comp-lecture/tiny-c-note1.html) 📘

## 🌟 プロジェクトのステータス
- ⭐ **90 Stars**
- 👀 **5 Watching**
- 🍴 **20 Forks**

## 🤝 コントリビューション
コントリビューション大歓迎！🐾
バグ修正や新機能の提案は、ぜひ [GitHub Issues](https://github.com/yui0/catc/issues) や Pull Request でお知らせください！

## 📜 ライセンス
このプロジェクトはオープンソースです。詳細はリポジトリのライセンスをご確認ください。📄

😺 **catc** でコンパイラの楽しさを体験しよう！
Happy Coding! 💻
