from lettuce import before, after, world
from common.functions import Functions


functions = Functions()


@before.all
def setup():
    world.remember = {}
    world.num_commands = 1
#TODO: Inicializar y parar el simulador de comandos
   

@after.all
def clean(total):
    functions.clean(world.remember)
