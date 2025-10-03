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

-- Apply controls over the pipeline
mpix.ctrl(mpix.cid.BLACK_LEVEL, 1)
mpix.ctrl(mpix.cid.RED_BALANCE, 1.8 * (1 << 10))
mpix.ctrl(mpix.cid.BLUE_BALANCE, 2.0 * (1 << 10))

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

-- Show the pipeline topology to inspect it before it runs
mpix.dump()

-- The pipeline will be run after the script returns
