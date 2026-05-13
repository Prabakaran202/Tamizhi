; ModuleID = 'tamizhi_engine'
source_filename = "tamizhi_engine"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@fmt = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %temp_res = alloca i32, align 4
  store i32 2, ptr %temp_res, align 4
  %load_val = load i32, ptr %temp_res, align 4
  %print_call = call i32 (ptr, ...) @printf(ptr @fmt, i32 %load_val)
  ret i32 0
}
