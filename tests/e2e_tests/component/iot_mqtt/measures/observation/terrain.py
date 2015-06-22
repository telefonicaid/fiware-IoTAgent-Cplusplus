from lettuce import before, after, world
from common.functions import Functions


functions = Functions()


@before.all
def setup():
    world.remember = {}
   

@after.all
def clean(total):
    functions.clean(world.remember)
