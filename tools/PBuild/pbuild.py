import click
import subprocess
from os import getcwd,mkdir,popen,chdir, path

print("PBuild For Pcrap.\n")

#>
stdlib = path.join("libraries", "*.c")
objs = path.join("objects", "*.c")
flags = path.join("src", "*.c") + " -o"

exe = path.join("bin" ,"Pcrap.exe")

home = path.expanduser('~')
rPath = path.join(home, "Pcrap")
#<

def CheckFor(cc_type):
    cmds = []
    if cc_type == 'tcc':
        cmds = [cc_type, '-v']
    elif cc_type == "gcc":
        cmds = [cc_type, '--version']

    process = subprocess.Popen(cmds,
                        stdout=subprocess.PIPE, 
                        stderr=subprocess.PIPE)

    stdout, stderr = process.communicate()
    if stderr:
        print("Error: Compiler not found.")
        return False
    else:
        print(f"{cc_type} found!")
        return True

def Compile(cc_type):
    print("Compiling Pcrap...")

    chdir(rPath)
    st = popen(f'{cc_type} {objs} {stdlib} {flags} {exe}')
    o = st.read()
    
    print(o)

    print("Building finished.")
    return


####
@click.group()
def cli():
    pass

@click.command()
@click.option('--cc-type', required=True,
                        type=click.Choice(['gcc', 'tcc'], case_sensitive=False))
def build(cc_type):
    click.echo(f"Building Pcrap with {cc_type}.")

    if CheckFor(cc_type) == False:
        return
        
    Compile(cc_type)

@click.command()
def download():
    if path.exists(rPath):
        click.echo(f"A Folder with the same name exists in {home}.")
        return #
    
    repo = "https://github.com/valkarias/PCrap.git" # lol.


    chdir(home)
    st = popen(f'git clone {repo}')
    o = st.read()
    
    print(o)


if __name__ == '__main__':
    cli.add_command(build)
    cli.add_command(download)

    cli()