import click
from utils import *

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
@click.option('--latest', is_flag=True)
def version(latest):
    if not latest:
        release = get_current_version()
        click.echo(f"Pa {release} on {platform.system()}")
        return

    latest_release = get_latest_release_name()
    if latest_release == False:
        print("\n")
        click.echo("Please try checking the repository instead.")
        click.echo("-> https://github.com/valkarias/Pa/releases")
        return
    
    click.echo(f"Latest Pa Version {latest_release}.")

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