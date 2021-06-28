import os
import subprocess
from colorama import init, Fore

init(autoreset=True)

clear = 0
not_clear = -1

supported = [
    "gcc",
    "tcc",
]

error = Fore.RED
reset = Fore.WHITE
finished = Fore.GREEN

def err(msg):
    print(f"{error}ERROR: {reset}{msg}")
    exit(not_clear)

def ok():
    print(f"{finished}FINISHED")
    exit(clear)

binDir = f"{os.getcwd()}\\bin"

objects = f"objects/*.c libraries/*.c"
flags = f"*.c -o"

exe = f"{binDir}\\Pcrap"

def run():
    string = input(">").strip()
    print()

    parsed = string.split(" ")
    
    parse(parsed)

def checkFor(compiler):
    #check if compiler is installed in the user's machine.
    command = ""

    if compiler == "gcc":
        command = "--version"
    elif compiler == "tcc":
        command = "-v"

    print(f"CHECKING FOR {compiler}")

    process = subprocess.Popen([compiler, command],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    
    stdout,stderr = process.communicate()

    if stderr:
        err("THE GIVEN COMPILER IS NOT INSTALLED IN YOUR MACHINE")
    elif stdout:
        print(f"{compiler} FOUND")

def validateCompiler(compiler):
    if not compiler in supported:
        err("UNKNOWN/UNSUPPORTED COMPILER")

    checkFor(compiler)

def parse(parsed):
    
    command = parsed[0]
    if len(parsed) == 1:
        err("INVALID INPUT")
    compiler = parsed[1]

    if command == "compiler":
        validateCompiler(compiler)
        build(compiler)
    else:
        err("UNKNOWN COMMAND")

def initBuild():
    if not os.path.exists(binDir):
        os.mkdir(binDir)


def execute(command):
    process = os.popen(command)
    output = process.read()

    print(output)

def build(compiler):
    initBuild()
    command = ""

    print("COMPILING PCRAP...")

    if compiler == "gcc":
        command = f"{compiler} {objects} {flags} {exe}"
    elif compiler == "tcc":
        command = f"{compiler} {objects} {flags} {exe}.exe"

    execute(command)
    ok()

run()