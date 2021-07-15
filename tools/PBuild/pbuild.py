import click
import subprocess
import os
import platform

#
home = os.path.expanduser('~')

objects = os.path.join(home, "PCrap", "objects", "*.c")
libraries = os.path.join(home, "PCrap", "libraries", "*.c")

source = os.path.join(home, "PCrap", "src", "*.c")
flags = "-o"

LINUX_BUILD = False

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

def execute(command):
    process = os.popen(command)
    output = process.read()

    print(output)

def compile(cc):
    os.chdir(os.path.join(home, "PCrap", "bin"))

    if LINUX_BUILD:
        print(f"{cc} {objects} {libraries} {source} {flags} -lm {exe}")
        execute( f"{cc} {objects} {libraries} {source} {flags} -lm {exe}" )
    else:
        execute( f"{cc} {objects} {libraries} {source} {flags} {exe}" )
    

@click.group()
def cli():
    pass


@click.command()
def download():
    repo = "https://github.com/valkarias/PCrap.git" # lmao

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
        printf("\n")
        click.secho("Building failed", fg='red')
        return
    
    click.echo(f"Compiling with {cc_type}.")
    compile(cc_type)

    click.secho("Building finished", fg='green')

if __name__ == '__main__':
    cli.add_command(download)
    cli.add_command(build)

    cli()