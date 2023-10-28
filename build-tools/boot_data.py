from io import SEEK_CUR, SEEK_SET
import re, sys, os, math

from elftools.elf.elffile import ELFFile
from elftools.elf.descriptions import describe_p_type

SECTOR_SIZE = 512

def find_symbol_in_map_file(map_file:str, symbol: str):
    with open(map_file, "r") as fmap:
        for line in fmap:
            if symbol in line:
                match = re.search("0x([0-9a-fA-F]+)", line)
                if match is not None:
                    return int(match.group(1), base=16)
    return None

def install_stage1(target: str, stage1: str, stage1_map: str, offset: int = 0, boot_data_lba: int = 0):
    phys = find_symbol_in_map_file(stage1_map, "phys")
    if phys is None:
        raise ValueError("Can't find __entry_start symbol in stage1.map")
    
    entry_offset = find_symbol_in_map_file(stage1_map, "__entry_start")
    if entry_offset is None:
        raise ValueError("Can't find __entry_start symbol in stage1.map")
    entry_offset -= phys

    boot_table_lba = find_symbol_in_map_file(stage1_map, "boot_table_lba")
    if boot_table_lba is None:
        raise ValueError("Can't find boot_table_lba symbol in stage1.map")
    boot_table_lba -= phys

    with open(stage1, "rb") as fstage1:
        with os.fdopen(os.open(target, os.O_WRONLY | os.O_CREAT), "wb+") as ftarget:
            ftarget.seek(offset * SECTOR_SIZE, SEEK_SET)

            # Write first 3 bytes jump instruction
            ftarget.write(fstage1.read(3))

            # Write starting at entry_offset (end of header)
            fstage1.seek(entry_offset - 3, SEEK_CUR)
            ftarget.seek(entry_offset - 3, SEEK_CUR)
            ftarget.write(fstage1.read())

            # Write location of boot_table_lba
            ftarget.seek(offset * SECTOR_SIZE + boot_table_lba, SEEK_SET)
            ftarget.write(boot_data_lba.to_bytes(4, byteorder="little"))

def addr_to_seg_offset(addr: int):
    seg = (addr & 0xFFFF0000) >> 4
    off = (addr & 0x0000FFFF)
    return (seg << 16) | off

def install_stage2(target: str, stage2: str, boot_data_lba: int = 0, offset: int = 0, limit: int = 0):
    with open(stage2, "rb") as fstage2:
        with os.fdopen(os.open(target, os.O_WRONLY | os.O_CREAT), "wb+") as ftarget:
            stage2_elf = ELFFile(fstage2)
            entry_point = addr_to_seg_offset(stage2_elf["e_entry"])

            boot_table = []
            current_lba = offset

            for segment in stage2_elf.iter_segments():
                if describe_p_type(segment["p_type"]) == "LOAD":
                    data = segment.data()
                    len_sectors = math.ceil(len(data) / SECTOR_SIZE)

                    # Add to boot table
                    boot_table.append({
                        "lba": current_lba,
                        "load_addr": addr_to_seg_offset(segment["p_vaddr"]),
                        "count": len_sectors
                        })

                    # Write data
                    ftarget.seek(current_lba * SECTOR_SIZE, SEEK_SET)
                    ftarget.write(data)
                    current_lba += len_sectors

                    if current_lba >= limit:
                        raise Exception(" > Stage2 is too big!!! It is overwriting partition 1.")

            # Add null table entry
            boot_table.append({
                "lba": 0,
                "load_addr": 0,
                "count": 0
                })
                    
            # Write boot data
            ftarget.seek(boot_data_lba * SECTOR_SIZE, SEEK_SET)
            ftarget.write(entry_point.to_bytes(4, byteorder="little"))

            for entry in boot_table:
                ftarget.write(entry["lba"].to_bytes(4, byteorder="little"))
                ftarget.write(entry["load_addr"].to_bytes(4, byteorder="little"))
                ftarget.write(entry["count"].to_bytes(2, byteorder="little"))

if __name__ == "__main__":
    if len(sys.argv) != 6:
        raise Exception()

    target = sys.argv[1]
    stage1 = sys.argv[2]
    stage1_map = sys.argv[3]
    stage2 = sys.argv[4]
    partition_offset = int(sys.argv[5])
    install_stage1(target, stage1, stage1_map, offset=partition_offset, boot_data_lba=1)
    install_stage2(target, stage2, offset=2, boot_data_lba=1, limit=partition_offset)
