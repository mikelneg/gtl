/*-------------------------------------------------------------

Copyright (c) 2016 Mikel Negugogor (http://github.com/mikelneg)
MIT license. See LICENSE.txt in project root for details.

---------------------------------------------------------------*/

// Dummy root signature

#define DummySignature ""                                                            \
                       "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT),             " \
                       "    CBV(b0),                                               " \
                       "    DescriptorTable( Sampler(s0) ),                        " \
                       "    DescriptorTable( SRV(t0, numDescriptors = 2),          " \
                       "                     UAV(u0) ),                            " \
                       "    RootConstants(num32BitConstants = 8, b0, space = 1),   " \
                       "    SRV(t1, space = 2),                                    " \
                       "    RootConstants(num32BitConstants = 1, b0, space = 2),   " \
                       "    SRV(t2, space = 2),                                    "
