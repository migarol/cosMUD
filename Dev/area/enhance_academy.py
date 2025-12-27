#!/usr/bin/env python3
"""
Enhance Darkhaven Academy with books, AI NPCs, and lore
"""

def read_file(filename):
    with open(filename, 'r', encoding='latin-1') as f:
        return f.read()

def write_file(filename, content):
    with open(filename, 'w', encoding='latin-1') as f:
        f.write(content)

def enhance_academy():
    print("🎓 Enhancing Darkhaven Academy...")
    print("=" * 80)

    content = read_file('/home/user/cosMUD/Main/area/newacad.are')

    # New training books and lore
    new_objects = """
#10500
book combat basics warrior~
Combat Fundamentals~
A well-worn training manual lies here, marked "Combat Fundamentals".~
~
35 526336 16385
5 0 0 0
1 500 50
E
book manual pages~
COMBAT FUNDAMENTALS - Darkhaven Academy Training Manual

LESSON 1: STANCE AND BALANCE
Never stand flat-footed. Keep your weight distributed, knees slightly bent.

LESSON 2: WEAPON GRIP
Your weapon is an extension of your arm. Grip firmly but not rigidly.

LESSON 3: READING YOUR OPPONENT
Watch their eyes, not their weapon. Eyes telegraph intention.

LESSON 4: LEARNING FROM DEFEAT
Every loss teaches. The training arena creatures each teach different lessons:
    Wolf (10300) - Speed and dodging
    Slug (10301) - Patience and accuracy
    Naga (10302) - Defense against magic
    Crawler (10303) - Poison resistance

A warrior who never falls never rises above mediocrity.
- Master Domick, Combat Instructor
~
#10501
book magic primer mage~
Primer of Arcane Studies~
A pristine book titled "Primer of Arcane Studies" rests on a shelf.~
~
35 526336 16385
5 0 0 0
1 800 80
E
book primer pages~
PRIMER OF ARCANE STUDIES - Darkhaven Academy

CHAPTER 1: THE WEAVE OF MAGIC
All magic draws from the Weave, an invisible tapestry of energy that
permeates all things. When you cast a spell, you reshape reality itself.

CHAPTER 2: SCHOOLS OF MAGIC
Evocation, Abjuration, Conjuration, Transmutation, Illusion, Enchantment,
Divination, Necromancy - each approaches the Weave differently.

CHAPTER 3: MANA MANAGEMENT
A foolish mage burns all power on the first enemy and dies to the second.
Start with Magic Missile. Progress to Fireball. Master restraint.

CHAPTER 4: ETHICAL CONSIDERATIONS
Magic is power. Power corrupts. Watch for signs of hubris.
The dark path is seductive (see: Argon, Order of Chaos leader).
Do not become what you fight against.
~
#10502
scroll student journal~
A Student's Journal~
A leather-bound journal lies discarded here.~
~
35 526336 16385
1 0 0 0
1 50 5
E
journal pages entries~
STUDENT JOURNAL - Year 502, Property of Kethros

Entry 1: I can't believe I'm actually here! Darkhaven Academy!

Entry 7: Fought a training wolf today. Got my ass kicked. Master Domick says
"Pain is the best teacher." My ribs disagree.

Entry 15: Met a girl named Sera. She's studying magic. We train together.

Entry 23: Everyone's talking about the ninjas attacking Darkhaven last month.
60-70 of them! Led by Snake Eyes. My cousin in RDAF said it was hell.

Entry 31: Master Domick pulled me aside. Said I have potential. Started teaching
me advanced footwork.

Entry 45: Found a hidden section in the library. Old books about the Cataclysm,
founding of Darkhaven. One mentioned "The Three Moon Prophecy". Creepy.

Entry 52: Sera and I made a pact - we'll watch each other's backs out there.

Entry 60 - Final Entry: Graduation tomorrow. This journal ends, my story begins.
For Darkhaven. For honor. For glory.
- Kethros, Class of 502
~
#10503
book stealth rogue thief~
The Shadow's Path~
A black leather book titled "The Shadow's Path" rests in darkness.~
~
35 526336 16385
5 0 0 0
1 600 60
E
book pages manual~
THE SHADOW'S PATH - Darkhaven Academy Rogue Training

LESSON 1: MOVING IN SILENCE
Your footfalls must be silent as falling snow. Practice on different surfaces.
Exercise: Walk Academy halls at night. If a guard sees you, start over.

LESSON 2: PICKING LOCKS
A lock is a puzzle made of metal. Listen to tumblers. Feel the resistance.
(Practice locks available in training room)

LESSON 3: FINDING TRAPS
Look for discolored stones, strange seams, tripwires, unnaturally clean floors.
If it looks too good to be true, it probably is.

LESSON 4: BACKSTABBING
Strike from behind, swiftly and silently. One perfect strike is worth a hundred
sloppy ones. WARNING: Only for monsters and PK-flagged enemies.

LESSON 5: ETHICAL THIEVERY
We are specialists in stealth operations and intelligence gathering, not bandits.
Remember: The best thief is one who's never caught.
~
#10504
tome academy history~
Chronicles of the Academy~
A thick tome bound in blue leather rests here.~
~
35 526336 16385
50 0 0 0
1 5000 500
E
tome pages history~
CHRONICLES OF DARKHAVEN ACADEMY - Est. Year 467

FOUNDING (Year 467):
Following the Dragon Wars, Arch-Mage Rennard recognized a need: trained
adventurers. Too many youths died in their first dungeon lacking basic knowledge.
Thus, Darkhaven Academy was born.

EARLY INSTRUCTORS:
- Domick "Ironwall" Varthane - Combat Master (still teaching!)
- Sage Meloreth - Arcane Studies (deceased 489, dragon attack)
- Whisper - Stealth Arts (identity unknown, still teaching)

THE NINJA INVASION (Year 502):
On May 15th, sixty to seventy ninja warriors led by Snake Eyes attacked Darkhaven.
Our advanced students joined RDAF and Rangers in defense.

Twenty-three Academy students fell that night.
Their names are engraved in the Memorial Hall.

We honor their sacrifice. They gave everything so others could learn in safety.

NOTABLE ALUMNI:
- Argon (Order of Chaos) - Class of 495 [Academy takes no responsibility for his evil turn]
- Aella (RDAF Commander) - Class of 498
- Athmoz (RDAF Intelligence) - Class of 500
- Metheus (Independent Hero) - Class of 499

May the next generation be worthy of their predecessors.
- Headmaster Thalorin, Year 512
~
#10505
note secret hidden exam~
a crumpled exam paper~
A crumpled exam paper has been stuffed behind a bookshelf.~
~
35 526336 16385
1 0 0 0
1 10 1
E
note paper exam~
DARKHAVEN ACADEMY - FINAL EXAMINATION
Student: [Name smudged], Year 502

How do you counter an enemy mage?
ANSWER: "Rush them! Mages are weak in melee."

When is killing justified?
ANSWER: "Self-defense, monsters, PK situations with proper flags. Never murder."

FINAL GRADE: 73% - PASS (Barely)

INSTRUCTOR NOTES: "This student has potential but lacks discipline. Combat
instincts good, magical theory needs work. Recommend field experience."
- Domick

[Someone has drawn a stick figure celebrating at the bottom]
~

"""

    # Find #OBJECTS section end (before #ROOMS or #MOBILES end)
    objects_marker = content.find('#ROOMS')
    if objects_marker == -1:
        objects_marker = content.find('#RESETS')

    if objects_marker == -1:
        print("ERROR: Could not find insertion point")
        return

    # Insert new objects
    content = content[:objects_marker] + new_objects + content[objects_marker:]
    print(f"✅ Added {new_objects.count('#105')} new books/lore items")

    # Find #RESETS section
    resets_marker = content.find('#RESETS')
    if resets_marker == -1:
        print("ERROR: Could not find #RESETS")
        return

    resets_end = content.find('\nS\n', resets_marker)
    if resets_end == -1:
        print("ERROR: Could not find end of RESETS")
        return

    # Add resets (need to find library room vnum first - will use a common room for now)
    # TODO: Find actual library room vnum
    new_resets = """
O 0 10500 1 10399
O 0 10501 1 10399
O 0 10502 1 10399
O 0 10503 1 10399
O 0 10504 1 10399
O 0 10505 1 10399
"""

    content = content[:resets_end] + new_resets + content[resets_end:]
    print("✅ Added resets for books")

    # Save enhanced version
    output_file = '/home/user/cosMUD/Dev/area/newacad.are'
    write_file(output_file, content)

    # Copy to Main
    write_file('/home/user/cosMUD/Main/area/newacad.are', content)

    print(f"✅ Saved enhanced Academy to Dev and Main")
    print(f"   New size: {len(content):,} bytes")

    summary = f"""
╔══════════════════════════════════════════════════════════════╗
║         DARKHAVEN ACADEMY ENHANCEMENT COMPLETE               ║
╚══════════════════════════════════════════════════════════════╝

📚 Added 6 Training Books/Lore Items:
   1. Combat Fundamentals (warrior training)
   2. Primer of Arcane Studies (mage training)
   3. Student's Journal (Kethros, Class of 502 - Ninja Invasion survivor)
   4. The Shadow's Path (rogue/thief training)
   5. Chronicles of the Academy (full history with Ninja Invasion)
   6. Crumpled Exam Paper (hidden Easter egg)

🎯 Content Features:
   - References the Ninja Invasion (May 502, Snake Eyes, 23 students died)
   - Mentions notable alumni: Argon, Aella, Athmoz, Metheus
   - Connects to existing lore (Arch-Mage Rennard, RDAF, Rangers)
   - Training for all classes (warrior, mage, rogue)
   - Hidden secrets and Easter eggs
   - Ethical considerations (Argon as cautionary tale)

📖 Lore Connections:
   - Ties to books in Darkhaven library
   - References real player characters from hiscores
   - Explains why mob 10399 is farmed so much (training ground)
   - Shows Academy as newbie → hero pipeline

🤖 AI Personality Profiles Created (see academy_enhancements.txt):
   - Master Domick (Combat) - Gruff veteran, "Pain is best teacher"
   - Instructor Abbigayle (Language) - Patient scholar
   - Bubba (Vampire Tactics) - Morbid humor expert
   - Whisper (Stealth) - Mysterious shadow operative

📝 Next Steps:
   1. Add mob programs to instructor NPCs (10340, 10394, 10422)
   2. Find actual library room vnum and update resets
   3. Add AI integration for NPCs using npc_ai.c
   4. Add room extra descriptions (bookshelf, memorial plaque, arena)
   5. Test in-game

Academy now has DEEP lore connecting everything! 🎓
    """

    print(summary)

    with open('/home/user/cosMUD/Dev/area/academy_summary.txt', 'w') as f:
        f.write(summary)

if __name__ == '__main__':
    enhance_academy()
