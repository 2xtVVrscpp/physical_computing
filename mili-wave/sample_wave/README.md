## 1. gpio_get
指定したGPIOピンの現在のデジタル入力状態を読み取る関数です。

- 引数: uint gpio
    - 状態を読み取りたいGPIOのピン番号を指定します。（例: プログラム内の PRESENCE_PIN）
- 返り値: bool
    - ピンの状態がHigh（通常3.3V）の場合は true (または1)
    - ピンの状態がLow（通常0V）の場合は false (または0)
- 処理内容:
    - 指定されたピンの現在の電圧レベルを読み取って論理値として返します。
    - コード内では if (gpio_get(PRESENCE_PIN)) のように使用されており、センサー等（ここでは PRESENCE_PIN）からの信号がHighかLowかを判定し、その結果に応じてLEDの処理を切り替えています。

## 2. gpio_put
指定したGPIOピンのデジタル出力状態（High / Low）を設定する関数です。

- 引数:
    - uint gpio: 状態を出力したいGPIOのピン番号。（例: LED_PIN）
    - bool value: 出力する値。true (または1など0以外の値) でHigh、false (または0) でLowを出力します。
- 返り値: なし (void)
- 処理内容:
    - 指定したGPIOピンの出力を指定したレベル（HighまたはLow）に駆動します。
    - この関数を使用する前に、プログラム内で行われているように gpio_set_dir(LED_PIN, GPIO_OUT); のように対象のピンを出力方向（GPIO_OUT）に設定しておく必要があります。
    - コード内では gpio_put(LED_PIN, 0) や gpio_put(LED_PIN, 1) としてLEDの点灯・消灯を制御しています。

## 3. uart_is_readable
UART（シリアル通信）で読み取り可能な受信データが存在するかどうかを確認する関数です。

- 引数: uart_inst_t *uart
    -使用するUARTのハードウェアインスタンスへのポインタを指定します。（例: uart0 や uart1。プログラム内ではマクロの UART_ID）
- 返り値: bool
    - 受信FIFO（バッファ）に読み取り可能なデータがある場合は true
    - データがない場合は false
- 処理内容:
    - プログラムの実行を停止（ブロック）することなく、受信バッファにデータが来ているかを即座に確認します。
    - コード内では while (uart_is_readable(UART_ID)) のように使用されており、「データを受信している間だけ」ループを回してデータを一気に読み取る安全な設計になっています。

## 4. uart_getc
UARTから1文字（1バイト）のデータを読み取る関数です。

- 引数: uart_inst_t *uart
    - 読み取りを行うUARTのインスタンスへのポインタを指定します。（例: UART_ID）
- 返り値: char
    - 受信した1バイトのデータを返します。（コード内では uint8_t 型の変数 ch に代入してバイナリデータとして扱っています）
- 処理内容:
    - UARTの受信FIFOから最初の1文字を取り出して返します。
    - 重要な特徴として、もし受信FIFOが空の場合、この関数は新しいデータが届くまでプログラムの実行を一時停止（ブロック）して待機します。
    - そのため、このコードのように事前に uart_is_readable() でデータが届いていることを確認した上で uart_getc() を呼び出すことで、予期せぬプログラムの停止（ハングアップのような状態）を防ぐことができます。




取得された値
> FD FC FB FA 08 FF 01 03 80 04 03 02 01 6E 02 8C 62 6E 02 8C 62 6E 02 8C 62 6E 02 8C 62 6E 02 8C 62 6E 02 69 62

FD FC FB FA: cmd header
08 FF 01 03 80 04 03 02 01; cmd returns config and status of sensor
6E 02 8C 62: status
6E 02 8C 62: status (repeat)
6E 02 8C 62: status (repeat)
6E 02 8C 62: status (repeat)
6E 02 8C 62: status (repeat)
6E 02 69 62: status (repeat)
6E 02: the distance between target (0x026E)
8C 62: sensitivity
