<pre>
<code>
     $ nmake
     $ bench.exe
</code>
</pre>

lambdaÇ…ëŒâûÇµÇΩ VC ÇÃ x64 î≈Ç™Ç¢ÇËÇ‹Ç∑ÅB(ëΩï™2010Ç∆2012)


Ç†Ç∆AVXëŒâûÇµÇƒÇ»Ç¢Ç∆é~Ç‹ÇÈãCÇ™Ç∑ÇÈÅB


i7-3770 ÇÃó·
<pre>
<code>
== latency/throughput ==
   reg64:       add:   latency: CPI=    1.01, IPC=    0.99
   reg64:       add:throughput: CPI=    0.35, IPC=    2.82
   reg64:      load:   latency: CPI=    5.00, IPC=    0.20
   reg64:      load:throughput: CPI=    0.63, IPC=    1.60
    m128:      pxor:   latency: CPI=    0.31, IPC=    3.20
    m128:      pxor:throughput: CPI=    0.31, IPC=    3.20
    m128:      padd:   latency: CPI=    1.00, IPC=    1.00
    m128:      padd:throughput: CPI=    0.53, IPC=    1.88
    m128:    pmuldq:   latency: CPI=    5.00, IPC=    0.20
    m128:    pmuldq:throughput: CPI=    1.01, IPC=    0.99
    m128:    loadps:   latency: CPI=    7.00, IPC=    0.14
    m128:    loadps:throughput: CPI=    0.50, IPC=    2.00
    m128:     xorps:   latency: CPI=    0.31, IPC=    3.20
    m128:     xorps:throughput: CPI=    0.31, IPC=    3.20
    m128:     addps:   latency: CPI=    3.00, IPC=    0.33
    m128:     addps:throughput: CPI=    1.01, IPC=    0.99
    m128:     mulps:   latency: CPI=    5.00, IPC=    0.20
    m128:     mulps:throughput: CPI=    1.01, IPC=    0.99
    m128:   blendps:   latency: CPI=    1.00, IPC=    1.00
    m128:   blendps:throughput: CPI=    0.53, IPC=    1.88
    m128:    pshufb:   latency: CPI=    1.00, IPC=    1.00
    m128:    pshufb:throughput: CPI=    0.53, IPC=    1.88
    m128:    pmullw:   latency: CPI=    5.00, IPC=    0.20
    m128:    pmullw:throughput: CPI=    1.01, IPC=    0.99
    m128:    phaddd:   latency: CPI=    2.00, IPC=    0.50
    m128:    phaddd:throughput: CPI=    1.53, IPC=    0.65
    m128:    pinsrd:   latency: CPI=    1.13, IPC=    0.89
    m128:    pinsrd:throughput: CPI=    1.13, IPC=    0.89
    m128:      dpps:   latency: CPI=   12.00, IPC=    0.08
    m128:      dpps:throughput: CPI=    2.03, IPC=    0.49
    m128:  cvtps2dq:   latency: CPI=    3.00, IPC=    0.33
    m128:  cvtps2dq:throughput: CPI=    1.01, IPC=    0.99
    m256:    loadps:   latency: CPI=    1.00, IPC=    1.00
    m256:    loadps:throughput: CPI=    1.00, IPC=    1.00
    m256:     xorps:   latency: CPI=    0.31, IPC=    3.20
    m256:     xorps:throughput: CPI=    0.31, IPC=    3.20
    m256:     mulps:   latency: CPI=    5.00, IPC=    0.20
    m256:     mulps:throughput: CPI=    1.02, IPC=    0.98
    m256:     addps:   latency: CPI=    3.00, IPC=    0.33
    m256:     addps:throughput: CPI=    1.01, IPC=    0.99
    m256:     divps:   latency: CPI=   18.02, IPC=    0.06
    m256:     divps:throughput: CPI=   14.54, IPC=    0.07
    m256:     divpd:   latency: CPI=   19.02, IPC=    0.05
    m256:     divpd:throughput: CPI=   16.46, IPC=    0.06
    m256:   rsqrtps:   latency: CPI=    7.00, IPC=    0.14
    m256:   rsqrtps:throughput: CPI=    2.03, IPC=    0.49
    m256:     rcpps:   latency: CPI=    7.01, IPC=    0.14
    m256:     rcpps:throughput: CPI=    2.03, IPC=    0.49
    m256:    sqrtps:   latency: CPI=   18.02, IPC=    0.06
    m256:    sqrtps:throughput: CPI=   14.54, IPC=    0.07
    m256:vperm2f128:   latency: CPI=    2.00, IPC=    0.50
    m256:vperm2f128:throughput: CPI=    1.06, IPC=    0.94
</code>
</pre>


i7-6770 ÇÃó·
<pre>
<code>
== latency/throughput ==
   reg64:       add:   latency: CPI=    1.00, IPC=    1.00
   reg64:       add:throughput: CPI=    0.28, IPC=    3.55
   reg64:      load:   latency: CPI=    5.00, IPC=    0.20
   reg64:      load:throughput: CPI=    0.63, IPC=    1.60
    m128:      pxor:   latency: CPI=    0.28, IPC=    3.55
    m128:      pxor:throughput: CPI=    0.28, IPC=    3.55
    m128:      padd:   latency: CPI=    1.00, IPC=    1.00
    m128:      padd:throughput: CPI=    0.33, IPC=    3.00
    m128:    pmuldq:   latency: CPI=    5.02, IPC=    0.20
    m128:    pmuldq:throughput: CPI=    0.63, IPC=    1.60
    m128:    loadps:   latency: CPI=    8.00, IPC=    0.12
    m128:    loadps:throughput: CPI=    0.50, IPC=    2.00
    m128:     xorps:   latency: CPI=    0.28, IPC=    3.55
    m128:     xorps:throughput: CPI=    0.28, IPC=    3.55
    m128:     addps:   latency: CPI=    4.00, IPC=    0.25
    m128:     addps:throughput: CPI=    0.50, IPC=    2.00
    m128:     mulps:   latency: CPI=    4.00, IPC=    0.25
    m128:     mulps:throughput: CPI=    0.50, IPC=    2.00
    m128:   blendps:   latency: CPI=    1.00, IPC=    1.00
    m128:   blendps:throughput: CPI=    0.33, IPC=    3.00
    m128:    pshufb:   latency: CPI=    1.00, IPC=    1.00
    m128:    pshufb:throughput: CPI=    1.01, IPC=    0.99
    m128:    pmullw:   latency: CPI=    5.00, IPC=    0.20
    m128:    pmullw:throughput: CPI=    0.63, IPC=    1.60
    m128:    phaddd:   latency: CPI=    3.00, IPC=    0.33
    m128:    phaddd:throughput: CPI=    2.00, IPC=    0.50
    m128:    pinsrd:   latency: CPI=    2.00, IPC=    0.50
    m128:    pinsrd:throughput: CPI=    2.00, IPC=    0.50
    m128:      dpps:   latency: CPI=   13.03, IPC=    0.08
    m128:      dpps:throughput: CPI=    1.93, IPC=    0.52
    m128:  cvtps2dq:   latency: CPI=    4.00, IPC=    0.25
    m128:  cvtps2dq:throughput: CPI=    0.50, IPC=    2.00
    m256:    loadps:   latency: CPI=    1.00, IPC=    1.00
    m256:    loadps:throughput: CPI=    0.50, IPC=    2.00
    m256:     xorps:   latency: CPI=    0.28, IPC=    3.55
    m256:     xorps:throughput: CPI=    0.28, IPC=    3.55
    m256:     mulps:   latency: CPI=    4.00, IPC=    0.25
    m256:     mulps:throughput: CPI=    0.50, IPC=    2.00
    m256:     addps:   latency: CPI=    4.01, IPC=    0.25
    m256:     addps:throughput: CPI=    0.50, IPC=    2.00
    m256:     divps:   latency: CPI=   11.00, IPC=    0.09
    m256:     divps:throughput: CPI=    5.00, IPC=    0.20
    m256:     divpd:   latency: CPI=   13.00, IPC=    0.08
    m256:     divpd:throughput: CPI=    8.00, IPC=    0.12
    m256:   rsqrtps:   latency: CPI=    4.00, IPC=    0.25
    m256:   rsqrtps:throughput: CPI=    1.00, IPC=    1.00
    m256:     rcpps:   latency: CPI=    4.00, IPC=    0.25
    m256:     rcpps:throughput: CPI=    1.00, IPC=    1.00
    m256:    sqrtps:   latency: CPI=   12.00, IPC=    0.08
    m256:    sqrtps:throughput: CPI=    6.00, IPC=    0.17
    m256:vperm2f128:   latency: CPI=    3.00, IPC=    0.33
    m256:vperm2f128:throughput: CPI=    1.00, IPC=    1.00
    m256:      pxor:   latency: CPI=    0.28, IPC=    3.55
    m256:      pxor:throughput: CPI=    0.28, IPC=    3.55
    m256:     paddd:   latency: CPI=    1.00, IPC=    1.00
    m256:     paddd:throughput: CPI=    0.33, IPC=    3.00
    m256:   vpermps:   latency: CPI=    3.00, IPC=    0.33
    m256:   vpermps:throughput: CPI=    1.00, IPC=    1.00
    m256:   vpermpd:   latency: CPI=    3.00, IPC=    0.33
    m256:   vpermpd:throughput: CPI=    1.00, IPC=    1.00
    m256:    vfmaps:   latency: CPI=    4.00, IPC=    0.25
    m256:    vfmaps:throughput: CPI=    0.50, IPC=    2.00
    m256:    vfmapd:   latency: CPI=    4.01, IPC=    0.25
    m256:    vfmapd:throughput: CPI=    0.50, IPC=    1.99
</code>
</pre>