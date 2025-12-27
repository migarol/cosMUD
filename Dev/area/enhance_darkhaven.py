#!/usr/bin/env python3
"""
Enhance Darkhaven area with new books, secrets, and interactive content
"""

import re

def read_file(filename):
    with open(filename, 'r', encoding='latin-1') as f:
        return f.read()

def write_file(filename, content):
    with open(filename, 'w', encoding='latin-1') as f:
        f.write(content)

def enhance_darkhaven():
    print("Reading original newdark.are...")
    content = read_file('/home/user/cosMUD/Main/area/newdark.are')

    # Find the #OBJECTS section end (before #ROOMS)
    objects_marker = content.find('#ROOMS')
    if objects_marker == -1:
        print("ERROR: Could not find #ROOMS marker")
        return

    # New objects to add
    new_objects = """
#21500
book history darkhaven~
the History of Darkhaven~
A leather-bound tome titled "The History of Darkhaven" rests on a shelf.~
~
35 526336 16385
50 0 0 0
1 50000 5000
E
book history pages~
THE HISTORY OF DARKHAVEN
Written by Arch-Mage Rennard, Year 512

Darkhaven stands as a beacon of civilization in these troubled lands. Founded
centuries ago by refugees fleeing the Cataclysm of the Three Moons, our city
has endured countless threats - from orc hordes to dragon raids, from planar
incursions to internal strife.

But none tested us more than the NINJA INVASION of May, Year 502.

They came in the night, sixty to seventy shadow warriors led by the infamous
Snake Eyes. Our RDAF defenders fought valiantly alongside the Rangers Guild.
The battle raged for three days. Many fell, but Darkhaven held.

Since then, an uneasy peace has settled. The OOC faction grows in power,
and rumors of Argon's infiltration tactics concern the Council. The Drow
Empire to the south plots in darkness, their Spider Queen Lolth ever hungry
for surface elf blood.

Yet we endure. We adapt. We survive.

For Darkhaven is not merely stone and mortar - it is the will of free people
refusing to bow to tyranny.

May these words guide future generations.

- Rennard, First Mage of the Tower
~
A
3 3
#21501
tome drow lolth spider~
Tome of the Spider Queen~
A sinister black tome bound in spider silk lies here, radiating malevolence.~
~
35 526336 20481
45 0 0 0
1 40000 4000
E
tome pages text~
THE SPIDER QUEEN'S DOMAIN
Forbidden Knowledge - Read at Your Peril

Lolth, Demon Queen of Spiders, rules the Drow with an iron fist. Her domains
are Spiders, Drow, Chaos, and Evil. She despises all surface-dwelling elves
with an eternal hatred that burns hotter than a thousand suns.

Only female Drow may worship her and gain her favor. She delights in watching
her followers fight for her attention, granting power to the most ruthless and
cunning. Betrayal, assassination, and intrigue please her dark heart.

Her followers say she created the Drow race itself, twisting them from their
surface kin in ages past. Whether this is truth or propaganda, none can say.

What IS certain: cross the Spider Queen and face a fate worse than death.
Her webs extend far beyond the Underdark. Her agents walk among us.

Trust no one.

[The rest of the pages appear blank, but you feel watched...]
~
A
26 -700
#21502
scroll ninja invasion~
a yellowed scroll~
A yellowed scroll tied with red ribbon lies here, its edges frayed.~
~
35 526336 16385
30 0 0 0
1 10000 1000
E
scroll text~
EMERGENCY DECREE - MAY 15, YEAR 502
Issued by the Darkhaven Council

CITIZENS OF DARKHAVEN:

At midnight last, our city came under ATTACK by forces unknown. Witnesses
report 60-70 ninja warriors led by one called "Snake Eyes" infiltrating our
defenses with preternatural stealth.

The RDAF has mobilized. The Rangers Guild stands with us. All able-bodied
citizens are hereby conscripted to defend the walls.

REMAIN INDOORS unless fighting. TRUST NO SHADOWS. Report suspicious
activity to the nearest guard immediately.

May the gods protect us all.

By order of the Council,
Sealed this day

[Below, in hasty scrawl]:
"They're in the Guild Hall! They killed Icingdale while I fought Snake Eye!
Fled to recall, HE FOLLOWED ME IN. Nowhere is safe. There are too many.
May Shaundakul guide our souls..." - Nezuke, final entry
~
#21503
journal rdaf classified~
RDAF Classified Journal~
A leather journal marked "RDAF - CLASSIFIED" sits here.~
~
35 526336 16385
40 0 0 0
1 30000 3000
E
journal pages~
RDAF INTELLIGENCE BRIEF - CLASSIFIED
Subject: OOC Infiltration Threat
Agent: Athmoz

Recent intelligence suggests the OOC faction is recruiting new PK characters
at an alarming rate. Their leader, Argon, appears to be orchestrating a
systematic infiltration of our ranks.

Recommendation: Increase vetting protocols. Watch for:
- New recruits with unusual combat skills
- Characters asking probing questions about RDAF operations
- Suspicious activity near our outpost (vnum 100006)

This is not paranoia. This is COLD WAR.

Stay vigilant. Trust, but verify.

For Darkhaven.

- Commander Aella, RDAF
~
#21504
book shaundakul exploration~
The Wayfarer's Codex~
A well-traveled book bound in weathered leather rests here.~
~
35 526336 16385
35 0 0 0
1 20000 2000
E
book codex pages~
THE WAYFARER'S CODEX
Prayers and Practices of Shaundakul, Rider of the Winds

Shaundakul, the god of travel and exploration, watches over those who seek
the unknown. Wherever people search for what is lost and that which is hidden,
Shaundakul is there.

His followers are adventurers, cartographers, treasure hunters, and wanderers.
He has no natural enemies, though he dislikes Vampires for their static,
parasitic nature. Paladins are not permitted in his faith - their rigid codes
conflict with the freedom of the road.

To honor Shaundakul:
- Never refuse to help a lost traveler
- Map your journeys and share your knowledge
- Seek the hidden and forgotten places
- Travel light, but carry tales

The wind is always at your back, Wayfarer.
~
#21505
book ancient prophecy~
Prophecies of the Ancients~
A mysterious tome with constantly shifting text lies here.~
~
35 526336 16417
60 0 0 0
1 100000 10000
E
book prophecy pages text~
PROPHECIES OF THE ANCIENTS
Translated from the Elder Tongue

When shadow warriors darken the haven's gates, [CHECK - Ninja Invasion, 502]
And spider silk weaves through noble halls,
The alliance of forest and order shall hold the line, [CHECK - RDAF+Rangers]
But beware the serpent that wears a friend's face. [???]

In the age when the GOD awakens to see all,
And memories linger in the minds of the simple folk,
The world itself shall breathe with new life,
As day and night dance their eternal cycle.

Three systems of power shall emerge:
The watchers who remember every slight,
The events that shake the foundations,
The rhythm of sun and moon.

[The text shifts and changes before your eyes...]

And when clans war for territory and honor,
When guards patrol lands claimed by blood,
When alliances form and crumble like sand,
Then shall the TRUE age of heroes begin.

Seek the secrets hidden in plain sight.
The fountain knows. The statue sees. The tome speaks.

[The final page is blank, waiting to be written...]
~
A
3 5
A
26 16777216
#21506
note secret hidden~
a crumpled note~
A small crumpled note has been tucked behind some books.~
~
35 526336 16385
1 0 0 0
1 100 10
E
note text~
For those clever enough to find this:

The fountain in the square holds more than water.
The statue's eyes follow the sun.
The library floor, three paces north of center, sounds hollow.
Behind the cathedral altar, shadows linger longer than they should.

Speak "mists" in the wrong place, and you'll find yourself elsewhere.

Knowledge is power. Secrets are survival.

- A Friend in the Shadows
~

"""

    # Insert new objects before #ROOMS
    content = content[:objects_marker] + new_objects + content[objects_marker:]
    print(f"✅ Added {new_objects.count('#21')} new objects")

    # Find #RESETS section
    resets_marker = content.find('#RESETS')
    if resets_marker == -1:
        print("ERROR: Could not find #RESETS marker")
        return

    # Find the 'S' that ends the RESETS section
    resets_end = content.find('\nS\n', resets_marker)
    if resets_end == -1:
        print("ERROR: Could not find end of RESETS section")
        return

    # New resets for library books
    new_resets = """
O 0 21500 1 21128
O 0 21501 1 21128
O 0 21502 1 21128
O 0 21503 1 21128
O 0 21504 1 21128
O 0 21505 1 21128
O 0 21506 1 21128
"""

    # Insert new resets before the 'S'
    content = content[:resets_end] + new_resets + content[resets_end:]
    print("✅ Added resets for library books")

    # Save enhanced version
    output_file = '/home/user/cosMUD/Dev/area/newdark.are'
    write_file(output_file, content)
    print(f"✅ Saved enhanced area to {output_file}")
    print(f"   Total size: {len(content)} bytes")

    # Create summary
    summary = f"""
╔══════════════════════════════════════════════════════════════╗
║           DARKHAVEN ENHANCEMENT COMPLETE                     ║
╚══════════════════════════════════════════════════════════════╝

📚 Added {new_objects.count('#21')} new readable books to library:
   - History of Darkhaven (Ninja Invasion lore)
   - Tome of the Spider Queen (Lolth lore)
   - Ninja Invasion Emergency Decree (Nezuke's last words)
   - RDAF Classified Journal (OOC infiltration)
   - The Wayfarer's Codex (Shaundakul worship)
   - Prophecies of the Ancients (meta/mysterious)
   - Secret Hidden Note (Easter egg hints)

🏛️ All books placed in Library (room #21128)

🎯 Players can now:
   - Read actual cosMUD history in-game
   - Discover secrets and Easter eggs
   - Learn deity lore (Lolth, Shaundakul)
   - Find hints about hidden content
   - Experience the Ninja Invasion through primary sources

📝 Next steps:
   - Add mob programs to NPCs
   - Create tavern (The Wandering Wayfarer)
   - Add interactive fountain & statue to square
   - Test in-game

File: {output_file}
Size: {len(content):,} bytes
    """

    print(summary)

    # Write summary to file
    write_file('/home/user/cosMUD/Dev/area/enhancement_summary.txt', summary)

if __name__ == '__main__':
    enhance_darkhaven()
