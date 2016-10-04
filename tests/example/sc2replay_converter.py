#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, sys, sc2reader
from sc2reader.events.game import *
from sc2reader.events.message import *
from sc2reader.events.tracker import *
import collections


def main(files, maxSecondsToConsider, windowCutInSeconds):
    actions_dic = collections.OrderedDict()  # id: action
    next_global_d = 1 # start to count action id at 1

    for filename in sc2reader.utils.get_files(files):
        replay = sc2reader.load_replay(filename, load_level=4, debug=True) # reads a folder
        #get all players
        players = [t.players for t in replay.teams]
        players = [y for x in players for y in x] # Not sure what this means?
        #get the matchup as a unique string (sorted by letters): e.g., ZZ TZ TTP and not ZT as Z>T in lex. ordering.
        matchup = [p.play_race[0:1] for p in players]
        matchup.sort()
        matchup= "".join(matchup)
        # get basics replays info
        replay_info = [matchup, replay.release_string, replay.type, replay.map_name, replay.start_time]

        if len(players) !=2: continue
        sec = 0
        for event in replay.events:
            if (event.second > maxSecondsToConsider): break
            if (isinstance(event, TargetPointCommandEvent) and event.has_ability):
                if (event.ability_name.startswith("Build")):

                    actionpos  = event.ability_name + "+"
                    actionneg  = event.ability_name + "-"
                    actiondone = actionpos if event.player.result[:1]=="W" else actionneg

                    if actionpos not in actions_dic:
                        actions_dic[actionneg] = next_global_d
                        next_global_d = next_global_d + 1
                        actions_dic[actionpos] = next_global_d
                        next_global_d = next_global_d + 1
                    
                    print(actions_dic[actiondone], end = " ")

                    hasChangedItemset = False
                    if (event.second - sec > windowCutInSeconds):
                        sec = event.second
                        print("-1", end=" ")
                        hasChangedItemset=True
                        
        if (not hasChangedItemset): print ("-1", end=" ")
        print("\t".join([str(s) for s in ["-2"]+replay_info]))

    # write the dico,  mapping integer -> action/build_order
    f = open("dico.txt", "w")
    for a in actions_dic:
        f.write("{1}\t{0}\n".format(actions_dic[a], a))
    f.close()


if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="""This program allows you to parse a replay or a directory of replays
        into sequences of itemsets representing the build order of each (player, game)
        with a few other info (matchup, winner, ...) that allows one to select the game
        he is interested in. These info must be removed after the selection in order
        for balancespance to work."""
    )
    parser.add_argument('FILE', type=str, help="A file or directory of replays")
    parser.add_argument('MAX_SECOND', type=int, help="Maximum number of seconds to consider of each replay")
    parser.add_argument('WINDOW_LENGTH', type=int, help="Length of the window for each itemset")
    args = parser.parse_args()
    main(args.FILE, args.MAX_SECOND, args.WINDOW_LENGTH)
