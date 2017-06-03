#pragma once

//
// 0x8809 used by challenge, logo palette/speed
//

// TODO: what game's this sample from?

// Call #1, at 0xac
// == Registers before ==
// AR0:   0001     AR1:   0010     AR2:   0020     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 00cb     AX0.L: 0000     AX1.H: 6573     AX1.L: 1000
// AC0.H: 0000     AC0.M: 00cb     AC0.L: bb00
// AC1.H: 0000     AC1.M: bb00     AC1.L: 0000
//
// dsp dmem write 0020 = 0000
// dsp dmem write 0020 = bbcb
//
// == Registers after ==
// AR0:   0001     AR1:   0010     AR2:   0021     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 00cb     AX0.L: 0000     AX1.H: 6573     AX1.L: 1000
// AC0.H: 0000     AC0.M: 00cb     AC0.L: bb00
// AC1.H: 0000     AC1.M: bbcb     AC1.L: 0000

// Call #2, at 0xc5
// == Registers before==
// AR0:   0002     AR1:   000f     AR2:   001f     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 0077     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0077     AC0.L: 2600
// AC1.H: 0000     AC1.M: 2600     AC1.L: 6573
//
// dsp dmem write 001f = 6573
// dsp dmem write 001f = 2677
//
// == Registers after ==
// AR0:   0002     AR1:   000f     AR2:   0020     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0000     IX3:   0000
// AX0.H: 0077     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: 0000     AC0.M: 0077     AC0.L: 2600
// AC1.H: 0000     AC1.M: 2677     AC1.L: 6573

// Call #3A at 0xe1
// 00d5 0086 0001 lri         $IX2, #0x0001
// ...
// 00e1 02bf 8809 call        0x8809
//
// == Registers before ==
// AR0:   0002     AR1:   000f     AR2:   0010     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0020     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0020     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0002     AC1.L: 0000
//
// dsp dmem write 0010 = 0000
// dsp dmem write 0011 = 0022
//
// == Registers after ==
// AR0:   0002     AR1:   000f     AR2:   0012     AR3:   0000
// IX0:   8000     IX1:   0000     IX2:   0001     IX3:   0000
// AX0.H: 0020     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0020     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0022     AC1.L: 0000

// Call #3B at 0x100
// 00f5 0086 0001 lri         $IX2, #0x0001
// ...
// 0100 02bf 8809 call        0x8809
// Dolphin / Crystal Chronicles
// == Registers before ==
// AR0:   0002     AR1:   002e     AR2:   0010     AR3:   0f45
// IX0:   8021     IX1:   ba38     IX2:   0001     IX3:   0000
// AX0.H: 0000     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0002     AC1.L: 0000
// PROD: 001000fffff00000
// SR: 3978
//
// dsp dmem write 0010 = 0000
// dsp dmem write 0011 = 0002
//
// == Registers after ==
// AR0:   0002     AR1:   002e     AR2:   0012     AR3:   0f45
// IX0:   8021     IX1:   ba38     IX2:   0001     IX3:   0000
// AX0.H: 0000     AX0.L: 6573     AX1.H: 6f64     AX1.L: 1000
// AC0.H: fff0     AC0.M: 0000     AC0.L: 0000
// AC1.H: 0000     AC1.M: 0002     AC1.L: 0000
// PROD: 001000fffff00000
// SR: 3960

bool Patch8809(u16* irom)
{
  return PatchAt(irom + 0x809, R"(
      ; AR2 is the only addressing register that corresponds to the dmem writes
      ; could be AC1.L or AX0.L in the second call, but can't be AX0.L in the
      ; third call.
      srr @$AR2, $AC1.L
      ; AC1.M after calling always look like either AC1.M | AC0.M or
      ; AC1.M | AX0.H. TODO: Why pick AX0.H?
      orr $AC1.M, $AX0.H
      ; the second dmem write is incremented only in calls #3A and #3B. There,
      ; IX2 is the only register set to 1, and it's specifically set to 1 in the
      ; ucode. It's set to 0 in the first two calls.
      addarn $AR2, $IX2
      ; obvious
      srri @$AR2, $AC1.M
      ret
    )");
}
