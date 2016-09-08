#!/usr/bin/env python
# -*- coding: utf-8 -*-

import argparse, sys, sc2reader
from sc2reader.events.game import *
from sc2reader.events.message import *
from sc2reader.events.tracker import *


def main():

    parser = argparse.ArgumentParser(
        description="""This program allows you to parse a replay or a directory of replys
        into sequences of itemsets representing the build order of each (player, game)
        with a few other info (matchup, winner, ...) that allows one to select the game
        he is interested in. These info must be removed after the selection in order
        for balancespance to work."""
    )
    parser.add_argument('FILE', type=str, help="A file or directory of replays")
    parser.add_argument('MAX_SECOND', type=int, help="Maximum number of seconds to consider of each replay")
    parser.add_argument('WINDOW_LENGTH', type=int, help="Length of the window for each itemset")
    args = parser.parse_args()



    actions_dic = {}  # id: action
    next_global_d = 1 # start to count action id at 1
    maxSecondsToConsider = args.MAX_SECOND # stop replay parsing after xxx game seconds
    windowCutInSeconds = args.WINDOW_LENGTH # change sequence itemset every xx seconds (cf. windowing)


    for filename in sc2reader.utils.get_files(args.FILE):
        replay = sc2reader.load_replay(filename, load_level=4, debug=True) # reads a folder
        #get all players
        players = [t.players for t in replay.teams]
        players = [y for x in players for y in x]
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
                    action=event.ability_name
                    if event.player.result[:1]=="W": action = action + "+"
                    else: action= action + "-"
                    if (action not in actions_dic):
                        actions_dic[action] = next_global_d
                        next_global_d = next_global_d + 1
                    print(actions_dic[action], end = " ")
                    hasChangedItemset = False
                    if (event.second - sec > windowCutInSeconds):
                        sec = event.second
                        print("-1", end=" ")
                        hasChangedItemset=True
        if (not hasChangedItemset): print ("-1", end=" ")
        print("\t".join([str(s) for s in ["-2"]+replay_info]))

    # write the dico,  mapping interger -> action/build_order
    f = open("dico.txt", "w")
    for a in actions_dic:
        f.write("{1}\t{0}\n".format(actions_dic[a], a))
    f.close()



if __name__ == '__main__':
    main()





    ''' todo for later, some infos that can help selects replays/games given their player infos

#parse the build order for each player
for playerIndex in range (1,len(players)+1):
    # first get the basic info of the player
    player_info = [ players[playerIndex-1].result, \
                    players[playerIndex-1].pick_race, \
                    players[playerIndex-1].play_race, \
                    players[playerIndex-1].region, \
                    players[playerIndex-1].subregion, \
                    players[playerIndex-1].toon_id, \
                    players[playerIndex-1].name ]
    '''
