#!/usr/bin/python3

def usage():
    import sys
    print("usage : compare-result.py a.csv b.csv")
    sys.exit(1)

class Result:
    def __init__(self):
        self.latency = {}
        self.throughput = {}


def load_csv(path, l_inst, t_inst):
    import csv

    result = Result()

    with open(path, 'r') as f:
        r = csv.DictReader(f)

        for row in r:
            lt = row['l/t']
            inst = row['inst']
            clas = row['class']

            if lt == 'latency':
                result.latency[(clas,inst)] = row
                l_inst[(clas,inst)] = True
            else:
                result.throughput[(clas,inst)] = row
                t_inst[(clas,inst)] = True

    return result

def dump_row(l_row, r_row, clas, inst):
    ratio = 'N/A'
    l_val = 'N/A'
    r_val = 'N/A'

    if l_row and r_row:
        l_ipc = float(l_row['ipc'])
        r_ipc = float(r_row['ipc'])

        l_cpi = float(l_row['cpi'])
        r_cpi = float(r_row['cpi'])

        ipc_ratio = (l_ipc / r_ipc)-1
        cpi_ratio = (l_cpi / r_cpi)-1

        print("%8s %32s | %7.2f-%-7.2f (%6.1f[%%]), %7.2f-%-7.2f (%6.1f[%%])"%
              (clas, inst,
               l_ipc, r_ipc, ipc_ratio * 100,
               l_cpi, r_cpi, cpi_ratio * 100))

    elif l_row:
        l_ipc = float(l_row['ipc'])
        r_ipc = 'N/A'

        l_cpi = float(l_row['cpi'])
        r_cpi = 'N/A'

        ipc_ratio = 'N/A'
        cpi_ratio = 'N/A'

        print("%32s | %7.2f-%-7s (%6s[%%]), %7.2f-%-7s (%6s[%%])"%
              (inst,
               l_ipc, r_ipc, ipc_ratio,
               l_cpi, r_cpi, cpi_ratio))

    elif r_row:
        l_ipc = 'N/A'
        r_ipc = float(r_row['ipc'])

        l_cpi = 'N/A'
        r_cpi = float(r_row['cpi'])

        ipc_ratio = 'N/A'
        cpi_ratio = 'N/A'

        print("%32s | %7s-%-7.2f (%6s[%%]), %7s-%-7.2f (%6s[%%])"%
              (inst,
               l_ipc, r_ipc, ipc_ratio,
               l_cpi, r_cpi, cpi_ratio))
        

def main():
    import csv
    import sys
    if (len(sys.argv) < 3):
        usage()

    left = sys.argv[1]
    right = sys.argv[2]

    l_list = {}
    t_list = {}

    l = load_csv(left, l_list, t_list)
    r = load_csv(right, l_list, t_list)

    print("============= LATENCY ==============================================================================")
    print("%8s %32s | %7s%8s (%6s[%%]), %7s%8s (%6s[%%])"%
          (' ',
           'instruction',
           'IPC',
           '',
           'rel',
           'CPI',
           '',
           'rel'))
    print("------------------------------------------+---------------------------------------------------------")

    l_list = list(l_list.keys())
    t_list = list(t_list.keys())

    l_list.sort()
    t_list.sort()

    for i in l_list:
        l_row = None
        r_row = None

        if i in l.latency:
            l_row = l.latency[i]
        if i in r.latency:
            r_row = r.latency[i]

        dump_row(l_row, r_row, i[0], i[1])

    print("\n")
    print("============= THROUGHPUT ===========================================================================")
    print("%8s %32s | %7s%8s (%6s[%%]), %7s%8s (%6s[%%])"%
          (' ',
           'instruction',
           'IPC',
           '',
           'rel',
           'CPI',
           '',
           'rel'))
    print("------------------------------------------+---------------------------------------------------------")
    for i in t_list:
        l_row = None
        r_row = None

        if i in l.throughput:
            l_row = l.throughput[i]
        if i in r.throughput:
            r_row = r.throughput[i]

        dump_row(l_row, r_row, i[0], i[1])

    

if __name__ == '__main__':
    main()