#!/usr/bin/env python3
"""
Complete World Analysis for cosMUD
Maps areas, connections, content density, and opportunities
"""

import re
import os
from collections import defaultdict

def analyze_area(filename):
    """Extract key info from an area file"""
    try:
        with open(filename, 'r', encoding='latin-1', errors='ignore') as f:
            content = f.read()
    except:
        return None

    info = {
        'name': '',
        'author': '',
        'vnums': set(),
        'rooms': 0,
        'mobs': 0,
        'objects': 0,
        'resets': 0,
        'mob_progs': 0,
        'readable_items': 0,
        'size': len(content),
        'flags': '',
        'connections': []
    }

    # Extract area name
    match = re.search(r'#AREA\s+(.+?)~', content)
    if match:
        info['name'] = match.group(1).strip()

    # Extract author
    match = re.search(r'#AUTHOR\s+(.+?)~', content)
    if match:
        info['author'] = match.group(1).strip()

    # Extract flags
    match = re.search(r'#FLAGS\s*\n(\d+)', content)
    if match:
        info['flags'] = match.group(1)

    # Count rooms and extract vnums
    room_section = re.search(r'#ROOMS(.+?)(?:#\w+|$)', content, re.DOTALL)
    if room_section:
        rooms = re.findall(r'^#(\d+)', room_section.group(1), re.MULTILINE)
        info['rooms'] = len(rooms)
        info['vnums'].update(rooms)

        # Find connections (exits to other areas)
        exits = re.findall(r'0 -1 (\d+)', room_section.group(1))
        info['connections'] = [int(x) for x in exits if x not in info['vnums']]

    # Count mobs
    mob_section = re.search(r'#MOBILES(.+?)#OBJECTS', content, re.DOTALL)
    if mob_section:
        mobs = re.findall(r'^#(\d+)', mob_section.group(1), re.MULTILINE)
        info['mobs'] = len([m for m in mobs if m != '0'])

        # Count mob programs
        info['mob_progs'] = len(re.findall(r'> \w+_prog', mob_section.group(1)))

    # Count objects
    obj_section = re.search(r'#OBJECTS(.+?)#ROOMS', content, re.DOTALL)
    if obj_section:
        objects = re.findall(r'^#(\d+)', obj_section.group(1), re.MULTILINE)
        info['objects'] = len([o for o in objects if o != '0'])

        # Count readable items (books, scrolls, notes)
        info['readable_items'] = len(re.findall(r'^(book|tome|scroll|note|journal|letter|parchment|tablet)',
                                                obj_section.group(1), re.MULTILINE | re.IGNORECASE))

    # Count resets
    reset_section = re.search(r'#RESETS(.+?)\nS\n', content, re.DOTALL)
    if reset_section:
        info['resets'] = len(reset_section.group(1).strip().split('\n'))

    return info

def main():
    area_dir = '/home/user/cosMUD/Main/area'
    areas = []

    print("🌍 Analyzing cosMUD World Structure...")
    print("=" * 80)

    # Read area.lst to get loaded areas
    with open(f'{area_dir}/area.lst', 'r') as f:
        loaded = [line.strip() for line in f if line.strip() and not line.startswith('$')]

    print(f"\n📋 Found {len(loaded)} loaded areas\n")

    # Analyze each area
    for filename in loaded:
        filepath = os.path.join(area_dir, filename)
        if os.path.exists(filepath):
            info = analyze_area(filepath)
            if info:
                info['filename'] = filename
                areas.append(info)

    # Sort by size
    areas.sort(key=lambda x: x['size'], reverse=True)

    # Print top areas
    print("🏆 TOP 20 LARGEST AREAS:")
    print(f"{'#':<3} {'Name':<40} {'Rooms':<6} {'Mobs':<6} {'Objs':<6} {'Progs':<6} {'Books':<6} {'Size':<8}")
    print("-" * 95)
    for i, area in enumerate(areas[:20], 1):
        print(f"{i:<3} {area['name'][:39]:<40} {area['rooms']:<6} {area['mobs']:<6} "
              f"{area['objects']:<6} {area['mob_progs']:<6} {area['readable_items']:<6} {area['size']//1024:>5}KB")

    # Identify key hubs
    print("\n\n🗺️  KEY WORLD HUBS:")
    print("-" * 80)
    hubs = [a for a in areas if a['name'] in ['New Darkhaven', 'Roads', 'Midgaard', 'New Darkhaven Academy']]
    for hub in hubs:
        print(f"\n{hub['name']} ({hub['filename']}):")
        print(f"  Rooms: {hub['rooms']}, Connections: {len(set(hub['connections']))}")
        print(f"  Mobs: {hub['mobs']}, Objects: {hub['objects']}")
        print(f"  Mob Programs: {hub['mob_progs']}, Readable Items: {hub['readable_items']}")

    # Identify areas with LOW content density
    print("\n\n⚠️  AREAS NEEDING ENHANCEMENT (Low Content Density):")
    print(f"{'Name':<40} {'Rooms':<6} {'Mobs':<6} {'Objs':<6} {'Density':<8}")
    print("-" * 80)

    for area in areas:
        if area['rooms'] > 0:
            density = (area['mobs'] + area['objects']) / area['rooms']
            if density < 1.5 and area['rooms'] > 10:  # Less than 1.5 items per room
                print(f"{area['name'][:39]:<40} {area['rooms']:<6} {area['mobs']:<6} "
                      f"{area['objects']:<6} {density:>6.2f}")

    # Identify areas with NO readable items
    print("\n\n📚 AREAS WITH NO BOOKS/LORE (Prime for Literature):")
    print("-" * 80)
    count = 0
    for area in areas:
        if area['rooms'] > 20 and area['readable_items'] == 0:
            print(f"  - {area['name']} ({area['rooms']} rooms)")
            count += 1
            if count >= 15:
                print(f"  ... and {len([a for a in areas if a['rooms'] > 20 and a['readable_items'] == 0]) - 15} more")
                break

    # Identify areas with NO mob programs
    print("\n\n🤖 AREAS WITH NO MOB PROGRAMS (Need AI/Life):")
    print("-" * 80)
    count = 0
    for area in areas:
        if area['rooms'] > 15 and area['mob_progs'] == 0 and area['mobs'] > 0:
            print(f"  - {area['name']} ({area['mobs']} mobs, 0 programs)")
            count += 1
            if count >= 15:
                print(f"  ... and {len([a for a in areas if a['rooms'] > 15 and a['mob_progs'] == 0 and a['mobs'] > 0]) - 15} more")
                break

    # Find clan-specific areas
    print("\n\n🏰 CLAN/GUILD SPECIFIC AREAS:")
    print("-" * 80)
    for area in areas:
        if 'guild' in area['name'].lower() or 'rdaf' in area['name'].lower() or 'order' in area['name'].lower():
            print(f"  - {area['name']}: {area['rooms']} rooms, {area['mobs']} mobs, "
                  f"{area['mob_progs']} programs")

    # Summary statistics
    print("\n\n📊 WORLD STATISTICS:")
    print("-" * 80)
    total_rooms = sum(a['rooms'] for a in areas)
    total_mobs = sum(a['mobs'] for a in areas)
    total_objects = sum(a['objects'] for a in areas)
    total_progs = sum(a['mob_progs'] for a in areas)
    total_books = sum(a['readable_items'] for a in areas)

    print(f"  Total Areas: {len(areas)}")
    print(f"  Total Rooms: {total_rooms:,}")
    print(f"  Total Mobs: {total_mobs:,}")
    print(f"  Total Objects: {total_objects:,}")
    print(f"  Total Mob Programs: {total_progs:,}")
    print(f"  Total Readable Items: {total_books}")
    print(f"\n  Average Rooms per Area: {total_rooms//len(areas)}")
    print(f"  Average Mobs per Area: {total_mobs//len(areas)}")
    print(f"  Average Programs per Area: {total_progs//len(areas)}")
    print(f"  Mob Program Coverage: {(sum(1 for a in areas if a['mob_progs'] > 0) / len(areas) * 100):.1f}%")
    print(f"  Book/Lore Coverage: {(sum(1 for a in areas if a['readable_items'] > 0) / len(areas) * 100):.1f}%")

    # Recommendations
    print("\n\n💡 ENHANCEMENT RECOMMENDATIONS:")
    print("=" * 80)
    print("\n1. HIGH PRIORITY - Major Cities/Hubs:")
    for area in areas[:10]:
        if area['rooms'] > 100:
            missing = []
            if area['mob_progs'] == 0:
                missing.append("mob programs")
            if area['readable_items'] == 0:
                missing.append("books/lore")
            if missing:
                print(f"   • {area['name']}: Add {', '.join(missing)}")

    print("\n2. MEDIUM PRIORITY - Guild/Clan Areas:")
    for area in areas:
        if ('guild' in area['name'].lower() or 'clan' in area['filename'].lower()):
            if area['readable_items'] == 0:
                print(f"   • {area['name']}: Add training manuals, guild history")

    print("\n3. LOW PRIORITY - Dungeon/Quest Areas:")
    print("   • Can add flavor text and hidden lore as time permits")

    print("\n\n✅ Analysis complete!")
    print("\nSuggested next steps:")
    print("  1. Enhance top 5 largest areas with mob programs + lore")
    print("  2. Add guild-specific content to all guild areas")
    print("  3. Create interconnecting content (rumors that reference other areas)")
    print("  4. Add hidden secrets and Easter eggs to major hubs")

if __name__ == '__main__':
    main()
