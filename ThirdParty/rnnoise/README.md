# RNNoise

- 取得元: https://github.com/xiph/rnnoise/tree/70f1d256acd4b34a572f999a05c87bf00b67730d
- ライセンス: [BSD 3-Clause](COPYING)
- 学習済みモデル: `models/rnnoise.bin`
  - `download_model.sh` で取得し、`src/write_weights.c` でバイナリにしたもの
  - `USE_WEIGHTS_FILE` を定義してビルドし、`rnnoise_model_from_buffer` で読み込む
- `src/rnnoise_data.c` は、モデルに付いてくる生成ファイルから `#ifndef USE_WEIGHTS_FILE` の部分 (重みの配列) を取り除いたもの
- 取得元から変えた箇所
  - `src/denoise.c`: `rnnoise_model_from_buffer` で `model->file` を初期化する (しないと `rnnoise_model_free` が不正なポインタを `fclose` して落ちる)
  - `src/parse_lpcnet_weights.c`: 層の大きさの掛け算を `size_t` にしてから行う (CodeQL の `cpp/integer-multiplication-cast-to-long` への対応)
  - `src/write_weights.c`: `fopen(..., "w")` を `"wb"` にする (Windows ではテキストモードで改行が変換され、モデルが壊れる)。ゲームのビルドには含めない
