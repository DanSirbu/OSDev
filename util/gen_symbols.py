# TODO, make this part of the toolchain

output = []
with open("util/out.txt") as f:
    for line in f:
        lineTokens = line.strip().replace('\t', " ").split(" ")
        
        if(len(lineTokens) < 4): # defines are also in here but we don't care
            continue
        
        addr = "0x" + lineTokens[0]
        output.append("{{ {0}, \"{1}\", \"{2}\" }},".format(addr, lineTokens[2], lineTokens[3]))

output.sort()

print("\n".join(output))