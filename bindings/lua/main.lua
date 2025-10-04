-- SPDX-License-Identifier: Apache-2.0

-- test that normal Lua works
io.stderr:write("hello world!\n")

-- generate a map of the mpix module
for k, v in pairs(mpix) do
  io.stderr:write(type(v)..":"..k.."\n")
  if type(v) == "table" then
    for kk, vv in pairs(v) do io.stderr:write("- "..kk.."\n") end
  end
end

-- Print the current image format
io.stderr:write("[format]\n")
for k, v in pairs(mpix.format()) do io.stderr:write("- "..k..": "..v.."\n") end

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

-- Compute a color from the current image
mpix.optimize_palette(1000)

-- Apply the palette
mpix.op.palette_encode(mpix.fmt.PALETTE8)
mpix.op.palette_decode()

-- The pipeline will be run after the script returns
