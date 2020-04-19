# Simple-Calculator-Assembly-code-generator

I2P(II) mini project 1 - Simple Calculator Assembly code generator

## 運行

- master, testing_master：多檔案架構，run solve.c
- submit, testing_submit：單檔案架構，run submit.c
- （單檔案架構中除 submit.c 有維護外，其他皆不保證版本及能否運行，反之亦然）

## 程式架構

### Struct

- `ErrorType`：（solve.c 預設）處理/指明各種 Error，方便 Debug。
- `BTNode`：（solve.c 預設）樹節點，加入`int weight`記錄運算權重。
- `Symbol`：（solve.c 預設）紀錄出現過的變數，加入`int assigned`記錄是否已被賦值。
- `Value`：整合數值與未知值，簡化含未知值的計算。
- `Register`：暫存器，`int occupied`記錄此暫存器是否已被占用。

### Variables

- （省略 solve.c 預設全域變數）
- `Register reg[]`：宣告 8 個暫存器。
- `int DEBUG_MODE`：切換 submit 與 debug 模式。（如果有要跑 code 可以改為 1，並將 line 864 的 inline 刪除，這是合併檔案時忘記改的 bug）
- `int reg_needed`：記錄計算此 statement 所需的 Register 數量。
- `int total_clock_cycle`：記錄使用的 clock cycle。
- `int last_clock_cycle`：記錄前次 clock cycle，以計算此 Statement 使用的 clock cycle。

### 主要函式功能

> （開發時切成 solve.c, lexer_parser.c, helpful.c, semantizer.c code_gen.c 等檔案撰寫，code 中有用註解標示，可用搜尋快速跳至。solve.c）

- `int main()`
  > 變數初始化。（其他原 solve.c 內容分別移至 lexer_parser.c 跟 helpful.c。）
- `void statement()`
  > 原 solve.c 函式，並將 parse tree 的後續處理分離至`evaluate(BTNode *)`中。
- `void evaluate(BTNode *)`
  > 分離 submit 跟 debug 的輸出。先將 parse tree 傳至`semantize()`做預處理，後傳至`codeGenerate()`產出 Assembly Code。
- `Value *semantize(BTNode *)`
  > 利用回傳值以計算數值，並更新運算權重。<br> Token 為 ID 時，特別判斷變數未賦值的狀況。<br> Token 為 Assign 時回傳`NULL`，以處理 multiple assign 的狀況。<br> Token 為 Ops 時，如果兩個子節點數值已知，則可優化為一個常數節點。
- `Register generateCode(BTNode *)`
  > 依照運算權重選擇優先執行的部分，以減少暫存器使用。
