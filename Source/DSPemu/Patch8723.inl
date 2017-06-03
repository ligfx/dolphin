#pragma once

//
// 0x8723 used by challenge
//

// Callpoint #1 at 0xae
// == Registers before ==
// AR0:   0001     AR1:   0010     AR2:   0021     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 00cb     AX0.L: 0000     AX1.H: 6573     AX1.L: 1000
// AC0.H: 0000     AC0.M: 00cb     AC0.L: bb00
// AC1.H: 0000     AC1.M: bbcb     AC1.L: 0000
//
// dsp dmem write 0021 = deb8
//
// == Registers after ==
// AR0:   0001     AR1:   0010     AR2:   0020     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 00cb     AX0.L: 0000     AX1.H: 6573     AX1.L: 1000
// AC0.H: 0000     AC0.M: 00cb     AC0.L: bb00
// AC1.H: 0000     AC1.M: deb8     AC1.L: 0000

// Callpoint #2 at 0xc7
// == Registers before ==
// AR0:   0002     AR1:   000f     AR2:   0020     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 0077     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0077     AC0.L: 2600
// AC1.H: 0000     AC1.M: 2677     AC1.L: 6573
//
// dsp dmem write 0020 = 4913
//
// == Registers after ==
// AR0:   0002     AR1:   000f     AR2:   001f     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 0077     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0077     AC0.L: 2600
// AC1.H: 0000     AC1.M: 4913     AC1.L: 6573

bool Patch8723(u16* irom)
{
  return PatchAt(irom + 0x723, R"(
      ; in GBA-HLE, the nonce challenge is XOR'd with 0x6f646573, which happens
      ; to match the values of the AX1.H register across these two calls.
      xorr $AC1.M, $AX1.H
      ; the value of @AR2 is always the same as AC1.M after, and no other
      ; register matches.
      srrd @$AR2, $AC1.M
      ret
  )");
}
