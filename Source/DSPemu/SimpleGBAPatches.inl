#pragma once

bool SimpleGBAPatches(u16* irom)
{
  // THESE ARE OBVIOUS
  // this is wait_for_dsp_mbox, but in another (the right?) spot
  if (!PatchAt(irom + 0x7e, R"(
      lrs $AC0.M, @DMBH
      andcf $AC0.M, #0x8000
      jlz 0x807e
      ret
    )"))
    return false;

  // this is wait_for_cpu_mbox, but in another (the right?) spot
  if (!PatchAt(irom + 0x78, R"(
      lrs $AC0.M, @CMBH
      andcf $AC0.M, #0x8000
      jlnz 0x8078
      ret
    )"))
    return false;

  // Four Swords Adventure / Dolphin / Nintendo ROM
  // STARTED 003f -> 80bc
  // == Registers ==
  // AR0:   0041     AR1:   0393     AR2:   0420     AR3:   0618
  // IX0:   8049     IX1:   85d0     IX2:   0000     IX3:   0000
  // AX0.H: 004b     AX0.L: 9b60     AX1.H: 0000     AX1.L: 0020
  // AC0.H: ffff     AC0.M: 004b     AC0.L: 0000
  // AC1.H: 0000     AC1.M: 0380     AC1.L: 0000
  // PROD: 0000000000001680
  // SR: 3870

  // 80be : dsp dmem write ffce = 004b // DSMAH
  // 80c0 : dsp dmem write ffcf = 9b60 // DSMAL
  // 80c4 : dsp dmem write ffc9 = 0000 // DSCR
  // 80c6 : dsp dmem write ffcd = 0000 // DSPA
  // 80c8 : dsp dmem write ffcb = 0020 // DSBL

  // *** ddma_in RAM (0x004b9b60) -> DRAM_DSP (0x0000) : size (0x00000020)
  // dsp dmem read ffc9 = 0000
  // STOPPED: 0041
  // == Registers ==
  // AR0:   0041     AR1:   0393     AR2:   0420     AR3:   0618
  // IX0:   8049     IX1:   85d0     IX2:   0000     IX3:   0000
  // AX0.H: 004b     AX0.L: 9b60     AX1.H: 0000     AX1.L: 0020
  // AC0.H: ffff     AC0.M: 0000     AC0.L: 0000
  // AC1.H: 0000     AC1.M: 0000     AC1.L: 0000
  // PROD: 0000000000001680
  // SR: 3864

  // Four Swords Adventure / Dolphin / patched
  // STARTED 003f -> 80bc
  // == Registers ==
  // AR0:   0041     AR1:   0393     AR2:   04f4     AR3:   0618
  // IX0:   8049     IX1:   85d0     IX2:   0000     IX3:   0000
  // AX0.H: 004b     AX0.L: 9b60     AX1.H: 0000     AX1.L: 0020
  // AC0.H: ffff     AC0.M: 004b     AC0.L: 0000
  // AC1.H: 0000     AC1.M: 0380     AC1.L: 0000
  // PROD: 00000000000003c0
  // SR: 3870

  // 80bd : dsp dmem write ffce = 004b
  // 80be : dsp dmem write ffcf = 9b60
  // 80c0 : dsp dmem write ffc9 = 0000
  // 80c1 : dsp dmem write ffcd = 0000
  // 80c2 : dsp dmem write ffcb = 0020
  // *** ddma_in RAM (0x004b9b60) -> DRAM_DSP (0x0000) : size (0x00000020)
  // 80c3 : dsp dmem read ffc9 = 0000

  // STOPPED: 0041
  // == Registers ==
  // AR0:   0041     AR1:   0393     AR2:   04f4     AR3:   0618
  // IX0:   8049     IX1:   85d0     IX2:   0000     IX3:   0000
  // AX0.H: 004b     AX0.L: 9b60     AX1.H: 0000     AX1.L: 0020
  // AC0.H: ffff     AC0.M: 0000     AC0.L: 0000
  // AC1.H: 0000     AC1.M: 0000     AC1.L: 0000
  // PROD: 00000000000003c0
  // SR: 3870

  // Seems to work
  // 0037 009b 0000 lri         $AX1.H, #0x0000 // DRAM addr
  // 0039 0099 0020 lri         $AX1.L, #0x0020 // length in bytes
  // 003b 0087 0000 lri         $IX3, #0x0000 // no ucode/iram upload
  // 003d 0080 0041 lri         $AR0, #0x0041 // return addr after DRAM upload
  // if (!PatchAt(irom + 0xbc, R"(
  //     srs @DSMAH, $AC0.M // or AC0.M
  //     srs @DSMAL, $AX0.L
  //     sr @DSCR, $IX3 // uhhh
  //     srs @DSPA, $AX1.H // correct
  //     srs @DSBL, $AX1.L // correct
  //     // lrs $AC0.M, @DSCR
  //     // lri $AC1.M, #0
  //     // andcf $AC0.M, #0x0004
  //     // jlz 0x80c2
  //     ; call wait_dma+#IROM_BASE
  //     jmpr $AR0
  //   )"))
  //   return false;

  // Seems to work
  if (!PatchAt(irom + 0x8b, R"(
      srs @DSMAH, $AX0.H
      srs @DSMAL, $AX0.L
      si @DSCR, #0x1
      srs @DSPA, $AX1.H
      srs @DSBL, $AX1.L
      lrs $AC0.M, @DSCR
    	andcf $AC0.M, #0x0004
    	jlz 0x8091
      ; call wait_dma+#IROM_BASE
      ret
    )"))
    return false;

  return true;
}
