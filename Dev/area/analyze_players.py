#!/usr/bin/env python3
"""
Analyze player equipment and activity to find key areas
"""

import re
import os
from collections import defaultdict, Counter

def analyze_player_file(filepath):
    """Extract equipment, kills, and activity from player file"""
    try:
        with open(filepath, 'r', encoding='latin-1', errors='ignore') as f:
            content = f.read()
    except:
        return None

    # Extract player info
    name_match = re.search(r'Name\s+(\S+)~', content)
    level_match = re.search(r'Level\s+(\d+)', content)
    clan_match = re.search(r'Clan\s+(.+?)~', content)
    mkills_match = re.search(r'MKills\s+(\d+)', content)

    if not name_match:
        return None

    player = {
        'name': name_match.group(1),
        'level': int(level_match.group(1)) if level_match else 0,
        'clan': clan_match.group(1) if clan_match else 'None',
        'mkills': int(mkills_match.group(1)) if mkills_match else 0,
        'equipment': [],
        'kills': [],
        'areas_visited': set()
    }

    # Extract equipped items
    objects = re.findall(r'#OBJECT\n(?:.*?\n)*?Vnum\s+(\d+)', content, re.MULTILINE)
    for vnum in objects:
        player['equipment'].append(int(vnum))
        # Determine area from vnum (first 2-3 digits)
        area_id = int(vnum) // 100
        player['areas_visited'].add(area_id)

    # Extract kills (shows which areas they've farmed)
    kills = re.findall(r'Killed\s+(\d+)\s+(\d+)', content)
    for vnum, count in kills:
        player['kills'].append((int(vnum), int(count)))
        area_id = int(vnum) // 100
        player['areas_visited'].add(area_id)

    return player

def get_area_name_from_vnum(vnum):
    """Try to determine area name from vnum"""
    area_map = {
        2: "Darkhaven/Newbie",
        21: "Darkhaven",
        100: "RDAF HQ",
        250: "Unknown Area 25",
        300: "Unknown Area 30",
        # Add more as we discover them
    }
    area_id = vnum // 100
    return area_map.get(area_id, f"Area {area_id}")

def main():
    print("🔍 Analyzing Player Equipment and Activity")
    print("=" * 80)

    # Get top players from hiscores
    top_players = [
        'Argon', 'Aella', 'Metheus', 'Exu', 'Nicremog', 'Trespin',
        'Athmoz', 'Andy', 'Maximus', 'Maelstrom', 'Garett'
    ]

    player_dir = '/home/user/cosMUD/Main/player'
    players_data = []

    # Analyze top players
    for name in top_players:
        letter = name[0].lower()
        filepath = os.path.join(player_dir, letter, name)
        if os.path.exists(filepath):
            data = analyze_player_file(filepath)
            if data:
                players_data.append(data)
                print(f"✓ Analyzed {name}")

    # Also scan for high-level players
    print("\n📊 Scanning all players for high-levels...")
    for letter_dir in os.listdir(player_dir):
        letter_path = os.path.join(player_dir, letter_dir)
        if os.path.isdir(letter_path) and len(letter_dir) == 1:
            for filename in os.listdir(letter_path):
                filepath = os.path.join(letter_path, filename)
                if os.path.isfile(filepath):
                    data = analyze_player_file(filepath)
                    if data and data['level'] >= 80 and data['name'] not in top_players:
                        players_data.append(data)

    print(f"\n✅ Analyzed {len(players_data)} players")

    # Aggregate equipment vnums
    equipment_counter = Counter()
    area_equipment_counter = Counter()

    for player in players_data:
        for vnum in player['equipment']:
            equipment_counter[vnum] += 1
            area_id = vnum // 100
            area_equipment_counter[area_id] += 1

    # Aggregate kills to find popular farming areas
    kill_counter = Counter()
    area_kill_counter = Counter()

    for player in players_data:
        for vnum, count in player['kills']:
            kill_counter[vnum] += count
            area_id = vnum // 100
            area_kill_counter[area_id] += count

    # Print results
    print("\n\n🏆 TOP 20 MOST EQUIPPED ITEMS (Unique Items):")
    print(f"{'Vnum':<8} {'Count':<6} {'Area':<30}")
    print("-" * 60)
    for vnum, count in equipment_counter.most_common(20):
        area_name = get_area_name_from_vnum(vnum)
        print(f"{vnum:<8} {count:<6} {area_name:<30}")

    print("\n\n🗺️  AREAS WITH MOST VALUED EQUIPMENT:")
    print(f"{'Area ID':<8} {'Items':<6} {'Approx Area':<30}")
    print("-" * 60)
    for area_id, count in area_equipment_counter.most_common(15):
        area_name = get_area_name_from_vnum(area_id * 100)
        print(f"{area_id:<8} {count:<6} {area_name:<30}")

    print("\n\n⚔️  TOP 15 MOST FARMED MOBS:")
    print(f"{'Vnum':<8} {'Kills':<8} {'Area':<30}")
    print("-" * 60)
    for vnum, kills in kill_counter.most_common(15):
        area_name = get_area_name_from_vnum(vnum)
        print(f"{vnum:<8} {kills:<8} {area_name:<30}")

    print("\n\n🎯 MOST POPULAR FARMING AREAS:")
    print(f"{'Area ID':<8} {'Total Kills':<12} {'Approx Area':<30}")
    print("-" * 60)
    for area_id, kills in area_kill_counter.most_common(15):
        area_name = get_area_name_from_vnum(area_id * 100)
        print(f"{area_id:<8} {kills:<12} {area_name:<30}")

    # Find clan affiliations
    print("\n\n🏰 CLAN DISTRIBUTION (Top Players):")
    clan_counter = Counter()
    for player in players_data:
        if player['clan'] != 'None':
            clan_counter[player['clan']] += 1

    for clan, count in clan_counter.most_common(10):
        print(f"  {clan}: {count} players")

    # Save detailed report
    report_file = '/home/user/cosMUD/Dev/area/player_analysis.txt'
    with open(report_file, 'w') as f:
        f.write("PLAYER EQUIPMENT AND ACTIVITY ANALYSIS\n")
        f.write("=" * 80 + "\n\n")

        f.write("TOP PLAYERS ANALYZED:\n")
        for p in sorted(players_data, key=lambda x: x['mkills'], reverse=True)[:20]:
            f.write(f"  {p['name']:<15} Lv{p['level']:<3} {p['clan']:<20} {p['mkills']:>6} kills\n")

        f.write(f"\n\nUNIQUE EQUIPMENT VNUMS: {len(equipment_counter)}\n")
        f.write(f"UNIQUE MOB VNUMS KILLED: {len(kill_counter)}\n")
        f.write(f"AREAS VISITED: {len(area_equipment_counter)}\n")

        f.write("\n\nKEY AREAS FOR ENHANCEMENT:\n")
        f.write("(Based on player activity and valued equipment)\n\n")

        for area_id, count in area_equipment_counter.most_common(10):
            area_name = get_area_name_from_vnum(area_id * 100)
            kills = area_kill_counter.get(area_id, 0)
            f.write(f"  Area {area_id} ({area_name}):\n")
            f.write(f"    - {count} players equipping items from here\n")
            f.write(f"    - {kills} total mob kills\n")
            f.write(f"    - STATUS: HIGH PRIORITY for enhancement\n\n")

    print(f"\n\n✅ Detailed report saved to: {report_file}")

    print("\n\n💡 RECOMMENDATIONS:")
    print("=" * 80)
    print("\n1. ENHANCE HIGH-TRAFFIC AREAS:")
    for area_id, _ in area_equipment_counter.most_common(5):
        print(f"   • Area {area_id} - Add lore, mob programs, secrets")

    print("\n2. IDENTIFY UNIQUE ITEMS:")
    print("   • Items equipped by multiple top players are clearly valued")
    print("   • Add backstory/lore to these items")

    print("\n3. CREATE ITEM LORE:")
    print("   • Add 'examine' descriptions to top 20 equipped items")
    print("   • Reference the mobs/areas they come from")

if __name__ == '__main__':
    main()
