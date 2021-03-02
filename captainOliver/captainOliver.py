# Captain Oliver Game

# Notes:
# Pick one of three items
# Certain items allow for you to go down certain paths
# Everything must be text based no pictures

class Player:
  def __init__(self):
    food = []
    weapons = []
    tools = []
    bag = []
    health = 100
    armor = 100


# Welcome message for start of game
def welcome_message():
  print("Hello, this is your captain speaking. You can call me Captain Oliver.")
  print("Our ship has crashed, and somehow we have ended up in this Labyrinth.")
  print("We must work together to find our way out....")

  choice = 0
  choices = [1,2]

  while choice not in choices:
    print("Here are your options:")
    print("   [1] Go back to ship for supplies")
    print("   [2] Adventure futher")
    choice = input("How do you want to proceed: ")
    if choice not in choices:
      print("Please enter the number for a valid option.")
  
  return choice

# Select first choice
def first_decision(choice):
  if choice == 1:
    go_back_to_ship()
  else if choice == 2:
    adventure_further()
  else
    print("SOMETHING WENT WRONG! PLEASE RESTART THE GAME")
    exit()

# Go back to ship for supplies
def go_back_to_ship():
  print("You make your way back to the ship. As you enter, the smell of the musty")
  print("sea soaked wood drifts through the wind. The moon casts just enough light")
  print("to illuminate the deck. Out of the dim beams you make out ")

# Adventure further

  
# Start of Game
playerChoice = welcome_message()

