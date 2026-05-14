#!/usr/bin/python3
"""
Show the difference between two snapshots of /proc/interrupts.
"""
import argparse

def decode_interrupts(text):
    lines = text.split('\n')
    cpu_labels = lines[0].split()
    result = {}
    for row in lines[1:]:
        if row.strip() == '':
            continue
        parts = row.split()
        row_id = parts[0]
        row_label = " ".join(parts[1+len(cpu_labels):])
        row_data = list(map(int, parts[1:1+len(cpu_labels)]))
        result[row_id] = {
            'data': row_data,
            'label': row_label
        }
    return result

def delta_interrupts(before, after):
    result = {}
    for key in before.keys():
        after_data = after[key]['data']
        before_data = before[key]['data']
        new_data = [after_data[i] - before_data[i] for i in range(len(after_data))]
        result[key] = {
            'data': new_data,
            'label': after[key]['label']
        }
    return result

def output_delta(delta, only_cpu=None):
    for key in sorted(delta.keys()):
        row = delta[key]['data']
        if only_cpu != None:
            row = [row[only_cpu] if only_cpu < len(row) else 0]
        if any(map(lambda x: x > 0, row)):
            print(f"{delta[key]['label']:26}: ", end="")
            if only_cpu:
                print(f"{row[0]}")
            else:
                for index, difference in enumerate(row):
                    if difference > 0:
                        print(f"CPU{index:2}:+{difference:-6} ", end="")
                print("")

def get_argparser():
    parser = argparse.ArgumentParser()
    parser.add_argument('--core', default=None, help='only report for specific core (by 0 based index)', type=int)
    parser.add_argument('before', type=argparse.FileType('r'))
    parser.add_argument('after', type=argparse.FileType('r'))
    return parser

if __name__ == '__main__':
    parser = get_argparser()
    args = parser.parse_args()
    before = decode_interrupts(args.before.read())
    after = decode_interrupts(args.after.read())
    delta = delta_interrupts(before, after)
    output_delta(delta, only_cpu=args.core)
