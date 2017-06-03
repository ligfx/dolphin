//
// 0x8458 used by joyoot length lo, right after (*0x81f4)(); AC1 << 16
//

// Callpoint #1 at 0x112
// Dolphin / Four Swords Adventures
//
// == Registers before ==
// AR0:   0002     AR1:   002f     AR2:   0012     AR3:   0015
// IX0:   8049     IX1:   85d0     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0070     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 829c     AC1.L: 0000
//
// dsp dmem write 0015 = 0000
// dsp dmem write 0016 = 82a3
//
// == Registers after ==
// AR0:   0002     AR1:   002f     AR2:   0012     AR3:   0017
// IX0:   8049     IX1:   85d0     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0000     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0000     AC1.L: 82a3

// dspemu / Four Swords Adventures
// == Registers before ==
// AR0:   0002     AR1:   0010     AR2:   0012     AR3:   0015
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0070     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 829c     AC1.L: 0000
// PROD: 0000000000070000
// SR: 3850
//
// dsp dmem write 0015 = 0000
// dsp dmem write 0016 = 82a3
//
// STOPPED: 0114
// == Registers after ==
// AR0:   0002     AR1:   0010     AR2:   0012     AR3:   0017
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0000     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0000     AC1.L: 82a3
// PROD: 0000000000070000
// SR: 3864

// Dolphin / Crystal Chronicles
// == Registers before ==
// AR0:   0002     AR1:   002f     AR2:   0012     AR3:   0015
// IX0:   8021     IX1:   ba38     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0070     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: e058     AC1.L: 0000
// PROD: 0000000000070000
// SR: 3970
//
// dsp dmem write 0015 = 0000
// dsp dmem write 0016 = e05f
//
// == Registers after ==
// AR0:   0002     AR1:   002f     AR2:   0012     AR3:   0017
// IX0:   8021     IX1:   ba38     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0000     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0000     AC1.L: e05f
// PROD: 0000000000070000
// SR: 3964

// Probably something similar to 0x81F4 ?

bool Patch8458(u16* irom)
{
  return PatchAt(irom + 0x458, R"(
      ; AC1.L after = AC1.M before + 7. this looks really stupid, but works
      ; in testing.
      addi $AC1.M, #0x7
      asr16 $ACC1
      ; AR3 gets incremented twice, and best matches the dmem writes
      srri @$AR3, $AC1.M ; or just #0x0.
      srri @$AR3, $AC1.L ; matches AC1.L after
      ret
  )");
}
