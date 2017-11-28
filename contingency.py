import sys
import numpy as np
from sets import Set
from ale_python_interface import ALEInterface


def getRAMVector(ale):
  ram_size = ale.getRAMSize()
  ram = np.zeros((ram_size,), dtype=np.uint8)
  ale.getRAM(ram)
  return ram


def initializeALE(romFile):
  ale = ALEInterface()

  ale.setInt("max_num_frames_per_episode", 18000)
  ale.setInt("random_seed", 123)
  ale.setFloat("repeat_action_probability", 0.0)
  ale.setInt("frame_skip", 5)

  random_seed = ale.getInt("random_seed")
  print("random_seed: " + str(random_seed))

  # Set USE_SDL to true to display the screen. ALE must be compilied
  # with SDL enabled for this to work. On OSX, pygame init is used to
  # proxy-call SDL_main.

  USE_SDL = False
  if USE_SDL:
    if sys.platform == 'darwin':
      import pygame
      pygame.init()
      ale.setBool('sound', False) # Sound doesn't work on OSX
    elif sys.platform.startswith('linux'):
      ale.setBool('sound', True)
    ale.setBool('display_screen', True)

  ale.loadROM(romFile)
  actionSet = ale.getMinimalActionSet()

  return ale, actionSet


if __name__ == "__main__":
  # Read arguments
  if(len(sys.argv) < 2):
    print("Usage ./ale_python_test1.py <ROM_FILE_NAME>")
    sys.exit()

  # Initialize ALE
  ale, actionSet = initializeALE(sys.argv[1])
  prevRAM = getRAMVector(ale)

  changing_bytes = Set([])
  len_changing_bytes = []
  contingent_bytes = Set([])
  len_contingent_bytes = []
  while not ale.game_over():
    # Try all actions 
    ram_diffs = []
    for actionIdx in xrange(actionSet.size):
      ale.saveState()
      reward = ale.act(actionSet[actionIdx]);
      currRAM = getRAMVector(ale)
      ram_diffs.append(prevRAM - currRAM)
      ale.loadState()

    # Test to see which bytes vary according to the action taken
    for i in xrange(ale.getRAMSize()): # Iterate over each byte
      contingent = False
      # Pairwise combination of different actions
      for j in xrange(len(ram_diffs)):
        for k in xrange(len(ram_diffs)):
          if j < k and int(ram_diffs[j][i]) - int(ram_diffs[k][i]) != 0:
            contingent = True
      if contingent:
        contingent_bytes.add(i)

    # Test to see which bytes vary, regardless of contingency
    for i in xrange(ale.getRAMSize()): # Iterate over each byte
      for j in xrange(len(ram_diffs)):
        if ram_diffs[j][i] != 0:
          changing_bytes.add(i)

    # Random walk
    a = actionSet[np.random.randint(actionSet.size)]
    reward = ale.act(a);
    prevRAM = currRAM

    # Keeping track of the evolving length
    len_contingent_bytes.append(len(contingent_bytes))
    len_changing_bytes.append(len(changing_bytes))


  import matplotlib.pylab as plt
  plt.title("Space Invaders")
  plt.ylabel('Set size')
  plt.xlabel('Time steps (w/ frame skip 5)')
  plt.plot(len_contingent_bytes, label='Contingent bytes')
  plt.plot(len_changing_bytes, label='Changing bytes')
  plt.legend()
  plt.savefig('graphs/space_invaders.png')