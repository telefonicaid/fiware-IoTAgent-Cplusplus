from lettuce import before, after, world
from common.user_steps import UserSteps


user_steps = UserSteps()


@before.all
def setup():
    world.remember = {}
    world.num_commands = 1
#TODO: Inicializar y parar el simulador de comandos
   

@after.all
def clean(total):
    user_steps.clean(world.remember)
