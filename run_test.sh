#!/bin/bash

if [ $# -eq 0 ]; then
  echo "Usage: $0 <testcase_group_name>"
  echo "Available testcase groups from config.json:"
  jq -r '.Groups[].GroupName' testcases/config.json | sed 's/^/  /'
  exit 1
fi

GROUP="$1"

# 检查 config.json 和 jq
if [ ! -e testcases/config.json ]; then
  echo "./testcases/config.json does not exist, please extract testcases to that directory."
  exit 1
fi
if ! command -v jq >/dev/null 2>&1; then
  echo 'You need the program "jq" to use this script.'
  echo 'Run "sudo apt install jq" to install it.'
  exit 1
fi

# 检查指定的测试组是否存在
if ! jq -e ".Groups[] | select(.GroupName == \"$GROUP\")" testcases/config.json >/dev/null; then
  echo "Testcase group '$GROUP' not found in config.json"
  echo "Available testcase groups:"
  jq -r '.Groups[].GroupName' testcases/config.json | sed 's/^/  /'
  exit 1
fi

# 拿到当前组里所有测试点的编号列表（假设都是数字或字符串 id）
TESTPOINTS=$(jq -r ".Groups[] | select(.GroupName == \"$GROUP\") | .TestPoints[]" testcases/config.json)

# 可执行文件名
EXENAME="code"
if [ ! -x "$EXENAME" ]; then
  echo "Please compile your program and make sure the executable is named '$EXENAME'"
  exit 1
fi

# 在当前路径下创建一个临时目录，放置可执行文件、临时生成的数据库等
function tmpdir() {
  if [ "$(uname -s)" = "Darwin" ]; then
    mktemp -d /tmp/ticket.XXXXXXXXXX
  else
    mktemp -d -p /tmp ticket.XXXXXXXXXX
  fi
}

TESTDIR="$(tmpdir)"
cp "$EXENAME" "$TESTDIR/"
EXE="$TESTDIR/$EXENAME"
CWD="$(pwd)"
BASEDIR="$CWD/testcases"

# 为日志（stdout / stderr）专门建一个目录
LOGDIR="$CWD/log"
mkdir -p "$LOGDIR"

cd "$TESTDIR"

# 每个测试点给 5 秒钟超时限制（你可以根据需要调整）
TIMEOUT_SEC=0.7

for TP in $TESTPOINTS; do
  # outfile 保存正常输出（stdout）
  outfile="$LOGDIR/test-${GROUP}-${TP}.out"
  # errfile 保存错误输出（stderr），包括 AddressSanitizer 信息
  errfile="$LOGDIR/test-${GROUP}-${TP}.err"

  echo "=== About to run test #$TP ==="

  # 用 timeout 限制每个测试点最多跑 TIMEOUT_SEC 秒
  # stdout 重定向到 $outfile，stderr 重定向到 $errfile
  timeout "${TIMEOUT_SEC}s" "$EXE" < "$BASEDIR/${TP}.in" > "$outfile" 2> "$errfile"
  STATUS=$?

  # 如果超时（timeout 返回 124），就在 errfile 末尾写一句提示
  if [ $STATUS -eq 124 ]; then
    echo "Error: Test #${TP} timed out after ${TIMEOUT_SEC}s" >> "$errfile"
    echo "Test #${TP} timed out. See error log: $errfile"
    # 退出脚本，不跑后续测试
    exit 1
  fi

  # 如果程序因为 AddressSanitizer 报错（如 segfault、heap-buffer-overflow 等）而崩溃
  # 那么 STATUS 通常是非 0（如 1、139 等），此时代码只要 STATUS != 0 且 !=124，就可以判定是崩溃
  if [ $STATUS -ne 0 ]; then
    echo "Error: Test #${TP} crashed (exit code $STATUS). See error log: $errfile"
    exit 1
  fi

  # 到这里说明程序正常返回。接着把正常输出和 expected .out 做对比
  difffile="$(mktemp -p /tmp diff.XXXXXXXXXX)"
  diff -ZB "$outfile" "$BASEDIR/${TP}.out" > "$difffile"
  DIFF_STATUS=$?

  if [ $DIFF_STATUS -eq 0 ]; then
    # 测试通过——删除 diff 文件和 errfile（因为 errfile 应该是空或没有错误）
    rm -f "$difffile" "$errfile"
    echo "Test #${TP} passed."
  else
    # 如果 output 不匹配，就打印前几行 diff 信息，并提示日志文件
    echo "Test #${TP} output mismatch. First few lines of diff:"
    head -n 5 "$difffile"
    echo "See full output: $outfile"
    echo "See expected: $BASEDIR/${TP}.out"
    echo "See diff: $difffile"
    echo "See errors (if any): $errfile"
    # 测试失败，退出
    exit 1
  fi
done

# 如果所有测试都通过
echo "=== All tests in group '$GROUP' completed successfully! ==="

# 清理临时目录
cd "$CWD"
rm -rf "$TESTDIR"
