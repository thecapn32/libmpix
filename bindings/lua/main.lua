-- SPDX-License-Identifier: Apache-2.0
-- Example ISP pipeline script in Lua

-- Add color correction operations
mpix.op.correct_black_level()
mpix.op.correct_white_balance()
mpix.op.kernel_denoise_3x3()

-- Apply controls over the pipeline
function q10(float) return math.ceil(float * (1 << 10)) end
mpix.ctrl(mpix.cid.BLACK_LEVEL, 10)
mpix.ctrl(mpix.cid.RED_BALANCE, q10(1.5))
mpix.ctrl(mpix.cid.BLUE_BALANCE, q10(1.9))

-- Test a few pixel format conversions
mpix.op.convert(mpix.fmt.RGB565)
mpix.op.convert(mpix.fmt.RGB24)
mpix.op.convert(mpix.fmt.YUYV)
mpix.op.convert(mpix.fmt.RGB24)

-- JPEG encode the result for convenience
mpix.op.jpeg_encode()

-- The pipeline will be run after the script returns
