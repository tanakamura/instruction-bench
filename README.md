# instruction-bench
Measures latency and throughputh for each instructions.

For precise measuring, running on Linux is strongly recommended.

     $ make
     $ ./bench

# Results
[Results](logs/linux/)

[i7-6700](logs/linux/Intel(R)Core(TM)i7-6700CPU@3.40GHz.csv)

[Ryzen7-3700X](logs/linux/AMDRyzen73700X8-CoreProcessor.csv)


## compare result

    $ python compare-result.py <result1.csv> <result2.csv>
