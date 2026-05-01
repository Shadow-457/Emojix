import json
import sys

def main():
    if len(sys.argv) < 3:
        print("Usage: generate.py <input.json> <output.h>")
        return

    with open(sys.argv[1], 'r', encoding='utf-8') as f:
        data = json.load(f)

    emojis = []
    for item in data:
        if 'emoji' in item and 'aliases' in item:
            emoji = item['emoji']
            for alias in item['aliases']:
                emojis.append((alias, emoji))

    # sort alphabetically or by length? By length descending might be good if we match suffixes
    # Actually, alphabet doesn't matter if we just check equality when searching.
    emojis.sort(key=lambda x: x[0])

    with open(sys.argv[2], 'w', encoding='utf-8') as f:
        f.write("#ifndef EMOJIS_H\n")
        f.write("#define EMOJIS_H\n\n")
        f.write("#include <stddef.h>\n\n")
        f.write("typedef struct {\n")
        f.write("    const char *alias;\n")
        f.write("    const char *emoji;\n")
        f.write("} EmojiMapping;\n\n")
        
        f.write(f"EmojiMapping EMOJI_MAPPINGS[{len(emojis)}] = {{\n")
        for alias, emoji in emojis:
            # Escape strings just in case
            alias_escaped = alias.replace('\\', '\\\\').replace('"', '\\"')
            emoji_escaped = emoji.replace('\\', '\\\\').replace('"', '\\"')
            f.write(f'    {{"..{alias_escaped}", "{emoji_escaped}"}},\n')
        f.write("};\n\n")
        f.write(f"const size_t EMOJI_MAPPINGS_COUNT = {len(emojis)};\n\n")
        f.write("#endif // EMOJIS_H\n")

if __name__ == "__main__":
    main()
