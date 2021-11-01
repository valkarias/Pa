import click
import subprocess
import os
import sys
import platform
import shutil
try:
    import requests
except:
    click.echo("Could not find the requests module...")
    click.echo("Installing the 'requests' module via pip..Hold on!")

    import importlib
    subprocess.check_call([sys.executable, "-m", "pip", "install", "requests"])
    print("\n")
    click.secho("Installing finished", fg='green')
    globals()["requests"] = importlib.import_module("requests")
import json

#
home = os.path.expanduser('~')

master = os.path.join(home, "PCrap")

objects = os.path.join(master, "objects", "*.c")
libraries = os.path.join(master, "libraries", "*.c")

source = os.path.join(master, "src", "*.c")
# >:)
opts = "-Ofast -flto"
flags = "-o"

LINUX_BUILD = False
REPO_NAME = "valkarias/PCrap"

exe = ""
if platform.system() == "Windows":
    exe = "pcrap.exe"
else:
    LINUX_BUILD = True
    exe = "pcrap"
#

def check():
    p = os.path.join(home, "PCrap")

    if os.path.exists(p) == False:
        click.secho("PCrap Directory is missing: ", fg='red')
        click.echo("Please use the 'download' command.")
        return -1

    elif os.path.isdir(p) == False:
        click.secho("A File with the same name exists: ", fg='red')
        click.echo(f"-> '{p}'")
        return -1

    if len(os.listdir(p)) == 0:
        click.secho("An Empty Directory with the same name exists: ", fg='red')
        click.echo("Please use the 'uninstall' command to clean up.")
        click.echo(f"-> '{p}'")
        return -1

    return 0

def validateCompiler(cc):
    commands = []

    click.echo(f"Checking for {cc}.")

    if cc == "gcc":
        commands = [cc, "--version"]
    else:
        commands = [cc, "-v"]

    process = subprocess.Popen(commands, 
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    
    stdout,stderr = process.communicate()

    if stderr:
        click.secho(f"Could not find '{cc}'.", fg='red')
        return False
    elif stdout:
        click.echo(f"{cc} found.")
    
    return True

def get_latest_release_name():
    try:
        req = requests.get(f"https://api.github.com/repos/{REPO_NAME}/releases")
    except Exception as e:
        click.secho(f"An unknown exception occured during request", fg='red')
        click.echo(f"-> {e}")
        return False
    trailing = [
        "-windows-latest",
        "-ubuntu-latest",
        "-macOS-latest"
    ]
    if req.status_code == 200:
        data = req.json()
        release_name = data[0]["name"]
        for i in trailing:
            if i in release_name:
                return release_name.replace(i, "")
    else:
        click.secho(f"Could not get the latest release from the repository", fg='red')
        click.echo(f"-> request status code: {req.status_code}")

    return False

def execute(command):
    process = os.popen(command)
    output = process.read()

    print(output)


def compile(cc):
    binp = os.path.join(master, "bin")
    os.chdir(binp)

    if LINUX_BUILD:
        execute( f"{cc} {objects} {libraries} {source} {opts} {flags} {exe} -lm" )
    else:
        execute( f"{cc} {objects} {libraries} {source} {opts} {flags} {exe}" )
    

@click.group()
def cli():
    pass


@click.command()
def download():
    repo = f"https://github.com/{REPO_NAME}.git" # lmao

    os.chdir(home)
    execute(
        f"git clone {repo}"
    )

    click.secho("Downloading finished", fg='green')


@click.command()
@click.option('--cc-type', required=True,
    type=click.Choice(['gcc', 'tcc'], case_sensitive=False))
def build(cc_type):
    if check() == -1:
        print("\n")
        click.secho("Building failed", fg='red')
        return

    if validateCompiler(cc_type) == False:
        print("\n")
        click.secho("Building failed", fg='red')
        return
    
    click.echo(f"Compiling with {cc_type}.")
    compile(cc_type)

    click.secho("Building finished", fg='green')

@click.command()
def version():
    release = get_latest_release_name()
    if release == False:
        print("\n")
        click.echo("Please try checking the repository instead.")
        click.echo("-> https://github.com/valkarias/PCrap/releases")
        return
    
    click.echo(f"Pcrap {release} on {platform.system()}")

@click.command()
def uninstall():
    p = os.path.join(home, "PCrap")
    
    if os.path.exists(p) == False:
        click.secho("PCrap Directory is missing", fg='red')
        return
    
    if (os.path.isdir(p)) and len(os.listdir(p)) == 0:
        os.rmdir(p)

        click.secho("Cleaning finished", fg='green')
        return
    
    shutil.rmtree(p, ignore_errors=True)
    click.secho("Uninstalling finished", fg='green')

if __name__ == '__main__':
    cli.add_command(version)
    cli.add_command(download)
    cli.add_command(build)
    cli.add_command(uninstall)

    cli()