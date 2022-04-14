import click
import subprocess
import os
import platform
import shutil
import json
import zipfile
from urllib.request import urlopen, Request, urlretrieve

#
home = os.path.expanduser('~')

master = os.path.join(home, "Pa")
master_c = os.path.join(master, "Pa-master")

objects = os.path.join(master_c, "objects", "*.c")
libraries = os.path.join(master_c, "libraries", "*.c")

source = os.path.join(master_c, "src", "*.c")
# >:)
opts = "-Ofast -flto"
flags = "-o"

LINUX_BUILD = False
WINDOWS_BUILD = False

REPO_NAME = "valkarias/Pa"

exe = ""
other_libraries = ""
if platform.system() == "Windows":
    WINDOWS_BUILD = True
    exe = "Pa.exe"
    other_libraries = os.path.join(os.environ['APPDATA'], "PA_LIBS")
else:
    LINUX_BUILD = True
    exe = "Pa"
    other_libraries = os.path.join(home, "PA_LIBS")
#

def check():
    p = os.path.join(home, "Pa")

    if os.path.exists(p) == False:
        click.secho("Pa Directory is missing: ", fg='red')
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
    url = f"https://api.github.com/repos/{REPO_NAME}/releases"

    req_class = Request(url, headers={"Accept": "application/json"})
    req = urlopen(req_class)
    trailing = [
        "-windows-latest",
        "-ubuntu-latest",
        "-macOS-latest"
    ]
    if req.status == 200:
        data = json.loads(req.read().decode())
        release_name = data[0]["name"]
        for i in trailing:
            if i in release_name:
                return release_name.replace(i, "")
    else:
        click.secho(f"Could not get the latest release from the repository", fg='red')
        click.echo(f"-> request status code: {req.status}")

    return False

def download_source():
    url = "https://github.com/valkarias/Pa/archive/master.zip"
    req, headers = urlretrieve(url, filename=master + ".zip")
    file = open(req)
    with zipfile.ZipFile(master + ".zip", 'r') as zip:
        extracted = os.path.join(home, "Pa")
        os.mkdir(extracted)
        click.echo("Extracting ZIP file..")
        try:
            zip.extractall(extracted)
        except:
            click.secho("Extraction failed.", fg='red')
            return False
    file.close()
    return True

def execute(command):
    process = os.popen(command)
    output = process.read()

    print(output)


def initLibs():
    click.echo("Moving libraries...")
    src = os.path.join(master_c, "libraries", "APIs")

    try:
        os.mkdir(other_libraries)
    except OSError:
        pass
    
    files = os.listdir(src)
    print("\n")
    for file in files:
        try:
            shutil.move(os.path.join(src, file), other_libraries)
            click.echo(f"Moved '{file}'")
        except:
            click.echo(f"'{file}' already exists")


def setPath():
    #There is no way you can set the actual path via pure python.
    #Only works on windows.
    binp = os.path.join(master_c, "bin")
    commands = ["setx", "/M", "path", f"{os.environ['Path']};{binp}"]

    process = subprocess.Popen(commands, 
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE)
    
    stdout,stderr = process.communicate()

    if stderr:
        click.secho(f"Could not modify the 'Path' environment variable.", fg='red')
    elif stdout:
        click.secho("Modified 'Path' environment variable!", fg="green")


def compile(cc):
    binp = os.path.join(master_c, "bin")
    os.chdir(binp)

    initLibs()

    command = f"{cc} {objects} {libraries} {source} {opts} {flags} {exe}"
    if LINUX_BUILD:
        execute(command + " -lm")
    else:
        execute(command)    

@click.group()
def cli():
    pass


@click.command()
def download():
    os.chdir(home)
    if not download_source():
        click.secho("Downloading failed", fg='red')
        return


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
    if WINDOWS_BUILD:
        setPath()

@click.command()
def version():
    release = get_latest_release_name()
    if release == False:
        print("\n")
        click.echo("Please try checking the repository instead.")
        click.echo("-> https://github.com/valkarias/Pa/releases")
        return
    
    click.echo(f"Pa {release} on {platform.system()}")

@click.command()
def uninstall():
    p = os.path.join(home, "Pa")
    
    if os.path.exists(p) == False:
        click.secho("Pa Directory is missing", fg='red')
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