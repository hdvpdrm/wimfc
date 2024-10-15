#!/bin/python3
#
# Script purpose is to retrieve the most frequent command from fish shell.
#

import os
import sys
import argparse

#script works only for fish shell and you won't run it under Windows
if os.name != "posix":
    print("wifmc error: script works under Unix-like OSes!")
    sys.exit(-1)
if "fish" not in os.environ["SHELL"]:
    print("wifmc error: script is suitable for fish only!")
    sys.exit(-1)

#parse CLI arguments
parser = argparse.ArgumentParser(prog="wifmc",
                                 description="Retrives the most frequent used command in fish shell",
                                 epilog="it is all about fun")
parser.add_argument("-c","--top",nargs="?",const=1,type=int,default=1,help="set the amount of commands that should be shown. Default is 1")
parser.add_argument("-s","--show-frequency",help="show the frequency of command. Don't do it by default",default=False,action="store_true")
args = parser.parse_args()

def get_history() -> str:
    '''returns the content of history file'''
    path = os.path.expanduser("~")+"/.local/share/fish/fish_history"
    if not os.path.exists(path):
        print("wifmc error: unable to locate fish history file!")
        sys.exit(-1)

    with open(path,"r") as f:
        return f.read()
def get_list_of_commands(history:str) -> list[str]:
    '''filters fish history file and returns only commands'''
    if not history:
        print("wifmc error: history file is empty!")
        sys.exit(-1)

    commands = filter(lambda s: "cmd" in s,history.split("\n"))
    filtered_commands = []
    for c in commands:
        filtered_commands.append(" ".join(c.split()[2::]))
    return filtered_commands

is_sudo = lambda c:"sudo" in c #convinient predicat
def process_sudo(line:list[str]):
    '''ignore sudo and if there is command after, then return it'''
    if len(line) == 1:
        return None
    else:
        return line[1]
        
def process_pipe(line:list[str]):
    '''extract all commands used in pipe, except sudo'''
    re_splitted = " ".join(line).split("|")
    commands = []
    for el in re_splitted:
        words = el.split()
        if is_sudo(el):
            el = process_sudo(words)
            if el is not None: commands.append(el)
        else:
            commands.append(words[0])
    return commands
def add_element(element:str, counter:dir):
    '''create or add element to the table of appearence'''
    if element in counter.keys():
        counter[element] += 1
    else:
        counter[element] = 1
    

def count_appearence_freq(commands: list[str]) -> dict:
    '''count frequency of command appearence and
    return the table where key is command and it's associated with its frequency '''
    counter = {}

    is_pipe = lambda c:"|" in c
    for c in commands:
        splitted = c.split()
        if is_pipe(splitted):#get all commands used in pipe
            commands = process_pipe(splitted)
            for c in commands:add_element(c,counter)
        elif is_sudo(splitted):#avoid sudo
            el = process_sudo(splitted)
            if el is not None: add_element(el,counter)
        else:
            #just regular command. Don't care of arguments
            add_element(splitted[0], counter)
    return counter


items = count_appearence_freq(
    get_list_of_commands(
        get_history()))
sorted_items = sorted(items.items(),key=lambda kv:(kv[1],kv[0]),reverse=True)

for i, item in enumerate(sorted_items):
    name, freq = item
    optional = freq if args.show_frequency else ""
    print("{} {} ".format(name,optional))
    if i == args.top-1: break
