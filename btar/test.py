#!/usr/bin/env python3


# File sizes to put in test archive

sizes = [
    1, 2, 3, 6, 30, 90, 128, 190, 200,
    404, 501, 999, 1023, 1024, 1025, 2048,
    3333, 4095, 4096, 4097, 8000, 12000,
    13000, 25000, 40000
]

create_files = '> 0.bytes ; '
pack_cmd = './btar pack rand.archive 0.bytes'
for i, size in enumerate(sizes):
    create_files += f'dd if=/dev/urandom of={i + 1}.bytes bs={size} count=1' + ' ; '
    pack_cmd += f' {i + 1}.bytes '


move_files = ''
for i in range(len(sizes) + 1):
    move_files += f'mv {i}.bytes {i}.bytes.old ; '

diff_files = ''
for i in range(len(sizes) + 1):
    diff_files += f'diff {i}.bytes {i}.bytes.old ; '

print(create_files)
print()
print(pack_cmd)
print()
print(move_files)
print()
print(diff_files)
