//
// 0x81f4 used by joyboot length and is apparently the end of some mixing
// function that is unforunately implemented differently in the Dolphin ROM (for
// an example of hitting this code from a different entry point, try connecting a
// GBA at the main menu of Metroid Prime).
//
// also, this is only a couple instructions long
// mix_add is at 81f9!

// Callpoint #1 at 0x10f
// Trace from Four Swords Adventure
// == Registers before ==
// AR0:   0002     AR1:   002e     AR2:   0012     AR3:   0013
// IX0:   8049     IX1:   85d0     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0070     AC0.L: 0000
// AC1.H: 0000     AC1.M: 829c     AC1.L: 0000
//
// dsp dmem write 0013 = 0000
// dsp dmem write 0014 = 829c
//
// == Registers after ==
// AR0:   0002     AR1:   002f     AR2:   0012     AR3:   0015
// IX0:   8049     IX1:   85d0     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0070     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0000     AC1.L: 829c
//
// Trace from dspemu / Four Swords Adventure
// == Registers before ==
// AR0:   0002     AR1:   000f     AR2:   0012     AR3:   0013
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0070     AC0.L: 0000
// AC1.H: 0000     AC1.M: 829c     AC1.L: 0000
// PROD: 001000fffff00000
// SR: 3870
//
// dsp dmem write 0013 = 0000
// dsp dmem write 0014 = 829c
//
// == Registers after ==
// AR0:   0002     AR1:   0010     AR2:   0012     AR3:   0015
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0070     AX0.L: 6573     AX1.H: 0070     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0000     AC1.L: 829c
// PROD: 0000000000070000
// SR: 3864

bool Patch81F4(u16* irom)
{
  // TODO: check other games / Metroid Prime to see what happens to AC1.H and
  // AC1.L before, and also to see what's written to @AR3.
  return PatchAt(irom + 0x1f4, R"(
      ; 1. AC1.L after = AC1.M before
      ; TODO: is AC1.H shifted to AC1.M in real code? could get a sample from metroid
      ; prime just for kicks
      ; 2. AR1 gets incremented
      asr16'ir $ACC1 : $AR1
      ; 1. AC0 after = 0
      ; 2. AR3 is the addressing register that best matches the dmem writes, and gets
      ; incremented twice.
      ; TODO: why write AC1.M? why not #0x0, or any other register?
      clr's $ACC0 : @$AR3, $AC1.M
      ; 1. AX1.H after = AX0.H before or AC0.M before. TODO: which one?
      ; also necessary for the following mul
      mrr $AX1.H, $AX0.H ; TODO: AX0.H or AC0.M?
      ; 1. make the product register match
      ; 2. store either AC1.L after shifting, or AC1.M before shifting. nbd.
      mul's $AX1.L, $AX1.H : @$AR3, $AC1.L
      ret
    )");
}
