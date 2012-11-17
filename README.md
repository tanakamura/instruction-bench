     $ nmake
     $ bench.exe

lambdaÇ…ëŒâûÇµÇΩ VC ÇÃ x64 î≈Ç™Ç¢ÇËÇ‹Ç∑ÅB(ëΩï™2010Ç∆2012)


Ç†Ç∆AVXëŒâûÇµÇƒÇ»Ç¢Ç∆é~Ç‹ÇÈãCÇ™Ç∑ÇÈÅB


i7-3770 ÇÃó·
    == latency/throughput ==
       reg64:       add:   latency: CPI=    0.89, IPC=    1.12
       reg64:       add:throughput: CPI=    0.32, IPC=    3.16
       reg64:      load:   latency: CPI=    4.46, IPC=    0.22
       reg64:      load:throughput: CPI=    0.54, IPC=    1.84
        m128:      pxor:   latency: CPI=    0.27, IPC=    3.67
        m128:      pxor:throughput: CPI=    0.27, IPC=    3.67
        m128:      padd:   latency: CPI=    0.87, IPC=    1.15
        m128:      padd:throughput: CPI=    0.46, IPC=    2.16
        m128:    pmuldq:   latency: CPI=    4.41, IPC=    0.23
        m128:    pmuldq:throughput: CPI=    0.88, IPC=    1.13
        m128:    loadps:   latency: CPI=    6.10, IPC=    0.16
        m128:    loadps:throughput: CPI=    0.44, IPC=    2.29
        m128:     xorps:   latency: CPI=    0.27, IPC=    3.67
        m128:     xorps:throughput: CPI=    0.27, IPC=    3.67
        m128:     addps:   latency: CPI=    2.62, IPC=    0.38
        m128:     addps:throughput: CPI=    0.88, IPC=    1.13
        m128:     mulps:   latency: CPI=    4.36, IPC=    0.23
        m128:     mulps:throughput: CPI=    0.88, IPC=    1.13
        m128:   blendps:   latency: CPI=    0.87, IPC=    1.15
        m128:   blendps:throughput: CPI=    0.46, IPC=    2.16
        m128:    pshufb:   latency: CPI=    0.87, IPC=    1.15
        m128:    pshufb:throughput: CPI=    0.46, IPC=    2.16
        m128:    pmullw:   latency: CPI=    4.36, IPC=    0.23
        m128:    pmullw:throughput: CPI=    0.96, IPC=    1.04
        m128:    phaddd:   latency: CPI=    1.74, IPC=    0.57
        m128:    phaddd:throughput: CPI=    1.33, IPC=    0.75
        m128:    pinsrd:   latency: CPI=    0.98, IPC=    1.02
        m128:    pinsrd:throughput: CPI=    0.98, IPC=    1.02
        m128:      dpps:   latency: CPI=   10.94, IPC=    0.09
        m128:      dpps:throughput: CPI=    1.77, IPC=    0.57
        m128:  cvtps2dq:   latency: CPI=    2.62, IPC=    0.38
        m128:  cvtps2dq:throughput: CPI=    0.88, IPC=    1.13
        m256:    loadps:   latency: CPI=    0.87, IPC=    1.15
        m256:    loadps:throughput: CPI=    0.87, IPC=    1.15
        m256:     xorps:   latency: CPI=    0.27, IPC=    3.67
        m256:     xorps:throughput: CPI=    0.27, IPC=    3.67
        m256:     mulps:   latency: CPI=    4.36, IPC=    0.23
        m256:     mulps:throughput: CPI=    0.88, IPC=    1.13
        m256:     addps:   latency: CPI=    2.68, IPC=    0.37
        m256:     addps:throughput: CPI=    0.91, IPC=    1.10
        m256:     divps:   latency: CPI=   15.71, IPC=    0.06
        m256:     divps:throughput: CPI=   12.68, IPC=    0.08
        m256:     divpd:   latency: CPI=   16.63, IPC=    0.06
        m256:     divpd:throughput: CPI=   14.43, IPC=    0.07
        m256:   rsqrtps:   latency: CPI=    6.10, IPC=    0.16
        m256:   rsqrtps:throughput: CPI=    1.77, IPC=    0.57
        m256:     rcpps:   latency: CPI=    6.10, IPC=    0.16
        m256:     rcpps:throughput: CPI=    1.77, IPC=    0.57
        m256:    sqrtps:   latency: CPI=   15.71, IPC=    0.06
        m256:    sqrtps:throughput: CPI=   12.68, IPC=    0.08
        m256:vperm2f128:   latency: CPI=    1.74, IPC=    0.57
        m256:vperm2f128:throughput: CPI=    1.16, IPC=    0.86
