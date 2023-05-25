/**
 * [TRIfA], Java part of Tox Reference Implementation for Android
 * Copyright (C) 2023 Zoff <zoff@zoff.cc>
 * <p>
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 * <p>
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * <p>
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA  02110-1301, USA.
 */

package com.zoffcc.applications.trifa;

import android.util.Log;

import java.util.Random;

public final class RandomNameGenerator
{
    private static final String TAG = "trifa.RndNameGen";

    // @formatter:off
    // Source: http://www.babynamewizard.com/the-top-1000-baby-names-of-2013-united-states-of-america
    private static final String[] firstNames = {
            "Sophia", "Emma", "Olivia", "Isabella", "Ava", "Mia", "Emily", "Abigail", "Madison", "Elizabeth", "Charlotte", "Avery", "Sofia", "Chloe", "Ella", "Harper", "Amelia", "Aubrey", "Addison", "Evelyn", "Natalie", "Grace", "Hannah", "Zoey", "Victoria", "Lillian", "Lily", "Brooklyn", "Samantha", "Layla", "Zoe", "Audrey", "Leah", "Allison", "Anna", "Aaliyah", "Savannah", "Gabriella", "Camila", "Aria", "Kaylee", "Scarlett", "Hailey", "Arianna", "Riley", "Alexis", "Nevaeh", "Sarah", "Claire", "Sadie", "Peyton", "Aubree", "Serenity", "Ariana", "Genesis", "Penelope", "Alyssa", "Bella", "Taylor", "Alexa", "Kylie", "Mackenzie", "Caroline", "Kennedy", "Autumn", "Lucy", "Ashley", "Madelyn", "Violet", "Stella", "Brianna", "Maya", "Skylar", "Ellie", "Julia", "Sophie", "Katherine", "Mila", "Khloe", "Paisley", "Annabelle", "Alexandra", "Nora", "Melanie", "London", "Gianna", "Naomi", "Eva", "Faith", "Madeline", "Lauren", "Nicole", "Ruby", "Makayla", "Kayla", "Lydia", "Piper", "Sydney", "Jocelyn", "Morgan", "Kimberly", "Molly", "Jasmine", "Reagan", "Bailey", "Eleanor", "Alice", "Trinity", "Rylee", "Andrea", "Hadley", "Maria", "Brooke", "Mariah", "Isabelle", "Brielle", "Mya", "Quinn", "Vivian", "Natalia", "Mary", "Liliana", "Payton", "Lilly", "Eliana", "Jade", "Cora", "Paige", "Valentina", "Kendall", "Clara", "Elena", "Jordyn", "Kaitlyn", "Delilah", "Isabel", "Destiny", "Rachel", "Amy", "Mckenzie", "Gabrielle", "Brooklynn", "Katelyn", "Laila", "Aurora", "Ariel", "Angelina", "Aliyah", "Juliana", "Vanessa", "Adriana", "Ivy", "Lyla", "Sara", "Willow", "Reese", "Hazel", "Eden", "Elise", "Josephine", "Kinsley", "Ximena", "Jessica", "Londyn", "Makenzie", "Gracie", "Isla", "Michelle", "Valerie", "Kylee", "Melody", "Catherine", "Adalynn", "Jayla", "Alexia", "Valeria", "Adalyn", "Rebecca", "Izabella", "Alaina", "Margaret", "Alana", "Alivia", "Kate", "Luna", "Norah", "Kendra", "Summer", "Ryleigh", "Julianna", "Jennifer", "Lila", "Hayden", "Emery", "Stephanie", "Angela", "Fiona", "Daisy", "Presley", "Eliza", "Harmony", "Melissa", "Giselle", "Keira", "Kinley", "Alayna", "Alexandria", "Emilia", "Marley", "Arabella", "Emerson", "Adelyn", "Brynn", "Lola", "Leila", "Mckenna", "Aniyah", "Athena", "Genevieve", "Allie", "Gabriela", "Daniela", "Cecilia", "Rose", "Adrianna", "Callie", "Jenna", "Esther", "Haley", "Leilani", "Maggie", "Adeline", "Hope", "Jaylah", "Amaya", "Maci", "Ana", "Juliet", "Jacqueline", "Charlie", "Lucia", "Tessa", "Camille", "Katie", "Miranda", "Lexi", "Makenna", "Jada", "Delaney", "Cassidy", "Alina", "Georgia", "Iris", "Ashlyn", "Kenzie", "Megan", "Anastasia", "Paris", "Shelby", "Jordan", "Danielle", "Lilliana", "Sienna", "Teagan", "Josie", "Angel", "Parker", "Mikayla", "Brynlee", "Diana", "Chelsea", "Kathryn", "Erin", "Annabella", "Kaydence", "Lyric", "Arya", "Madeleine", "Kayleigh", "Vivienne", "Sabrina", "Cali", "Raelynn", "Leslie", "Kyleigh", "Ayla", "Nina", "Amber", "Daniella", "Finley", "Olive", "Miriam", "Dakota", "Elliana", "Juliette", "Noelle", "Alison", "Amanda", "Alessandra", "Evangeline", "Phoebe", "Bianca", "Christina", "Yaretzi", "Raegan", "Kelsey", "Lilah", "Fatima", "Kiara", "Elaina", "Cadence", "Nyla", "Addyson", "Giuliana", "Alondra", "Gemma", "Ashlynn", "Carly", "Kyla", "Alicia", "Adelaide", "Laura", "Allyson", "Charlee", "Nadia", "Mallory", "Heaven", "Cheyenne", "Ruth", "Tatum", "Lena", "Ainsley", "Amiyah", "Journey", "Malia", "Haylee", "Veronica", "Eloise", "Myla", "Mariana", "Jillian", "Joanna", "Madilyn", "Baylee", "Selena", "Briella", "Sierra", "Rosalie", "Gia", "Briana", "Talia", "Abby", "Heidi", "Annie", "Jane", "Maddison", "Kira", "Carmen", "Lucille", "Harley", "Macy", "Skyler", "Kali", "June", "Elsie", "Kamila", "Adelynn", "Arielle", "Kelly", "Scarlet", "Rylie", "Haven", "Marilyn", "Aubrie", "Kamryn", "Kara", "Hanna", "Averie", "Marissa", "Jayda", "Jazmine", "Camryn", "Everly", "Jazmin", "Lia", "Karina", "Maliyah", "Miley", "Bethany", "Mckinley", "Jayleen", "Esmeralda", "Macie", "Aleah", "Catalina", "Nayeli", "Daphne", "Janelle", "Camilla", "Madelynn", "Kyra", "Addisyn", "Aylin", "Julie", "Caitlyn", "Sloane", "Gracelyn", "Elle", "Helen", "Michaela", "Serena", "Lana", "Angelica", "Raelyn", "Nylah", "Karen", "Emely", "Bristol", "Sarai", "Alejandra", "Brittany", "Vera", "April", "Francesca", "Logan", "Rowan", "Skye", "Sasha", "Carolina", "Kassidy", "Miracle", "Ariella", "Tiffany", "Itzel", "Justice", "Ada", "Brylee", "Jazlyn", "Dahlia", "Julissa", "Kaelyn", "Savanna", "Kennedi", "Anya", "Viviana", "Cataleya", "Jayden", "Sawyer", "Holly", "Kaylie", "Blakely", "Kailey", "Jimena", "Melany", "Emmalyn", "Guadalupe", "Sage", "Annalise", "Cassandra", "Madisyn", "Anabelle", "Kaylin", "Amira", "Crystal", "Elisa", "Caitlin", "Lacey", "Rebekah", "Celeste", "Danna", "Marlee", "Gwendolyn", "Joselyn", "Karla", "Joy", "Audrina", "Janiyah", "Anaya", "Malaysia", "Annabel", "Kadence", "Zara", "Imani", "Maeve", "Priscilla", "Phoenix", "Aspen", "Katelynn", "Dylan", "Eve", "Jamie", "Lexie", "Jaliyah", "Kailyn", "Lilian", "Braelyn", "Angie", "Lauryn", "Cynthia", "Emersyn", "Lorelei", "Monica", "Alanna", "Brinley", "Sylvia", "Journee", "Nia", "Aniya", "Breanna", "Fernanda", "Lillie", "Amari", "Charley", "Lilyana", "Luciana", "Raven", "Kaliyah", "Emilee", "Anne", "Bailee", "Hallie", "Zariah", "Bridget", "Annika", "Gloria", "Zuri", "Madilynn", "Elsa", "Nova", "Kiley", "Johanna", "Liberty", "Rosemary", "Aleena", "Courtney", "Madalyn", "Aryanna", "Tatiana", "Angelique", "Harlow", "Leighton", "Hayley", "Skyla", "Kenley", "Tiana", "Dayana", "Evelynn", "Selah", "Helena", "Blake", "Virginia", "Cecelia", "Nathalie", "Jaycee", "Danica", "Dulce", "Gracelynn", "Ember", "Evie", "Anika", "Emilie", "Erica", "Tenley", "Anabella", "Liana", "Cameron", "Braylee", "Aisha", "Charleigh", "Hattie", "Leia", "Lindsey", "Marie", "Regina", "Isis", "Alyson", "Anahi", "Elyse", "Felicity", "Jaelyn", "Amara", "Natasha", "Samara", "Lainey", "Daleyza", "Miah", "Melina", "River", "Amani", "Aileen", "Jessie", "Whitney", "Beatrice", "Caylee", "Greta", "Jaelynn", "Milan", "Millie", "Lea", "Marina", "Kaylynn", "Kenya", "Mariam", "Amelie", "Kaia", "Maleah", "Ally", "Colette", "Elisabeth", "Dallas", "Erika", "Karlee", "Alayah", "Alani", "Farrah", "Bria", "Madalynn", "Mikaela", "Adelina", "Amina", "Cara", "Jaylynn", "Leyla", "Nataly", "Braelynn", "Kiera", "Laylah", "Paislee", "Desiree", "Malaya", "Azalea", "Kensley", "Shiloh", "Brenda", "Lylah", "Addilyn", "Amiya", "Amya", "Maia", "Irene", "Ryan", "Jasmin", "Linda", "Adele", "Matilda", "Emelia", "Emmy", "Juniper", "Saige", "Ciara", "Estrella", "Jaylee", "Jemma", "Meredith", "Myah", "Rosa", "Teresa", "Yareli", "Kimber", "Madyson", "Claudia", "Maryam", "Zoie", "Kathleen", "Mira", "Paityn", "Isabela", "Perla", "Sariah", "Sherlyn", "Paola", "Shayla", "Winter", "Mae", "Simone", "Laney", "Pearl", "Ansley", "Jazlynn", "Patricia", "Aliana", "Brenna", "Armani", "Giana", "Lindsay", "Natalee", "Lailah", "Siena", "Nancy", "Raquel", "Willa", "Lilianna", "Frances", "Halle", "Janessa", "Kynlee", "Tori", "Leanna", "Bryanna", "Ellen", "Alma", "Lizbeth", "Wendy", "Chaya", "Christine", "Elianna", "Mabel", "Clarissa", "Kassandra", "Mollie", "Charli", "Diamond", "Kristen", "Coraline", "Mckayla", "Ariah", "Arely", "Blair", "Edith", "Joslyn", "Hailee", "Jaylene", "Chanel", "Alia", "Reyna", "Casey", "Clare", "Dana", "Alena", "Averi", "Alissa", "Demi", "Aiyana", "Leona", "Kailee", "Karsyn", "Kallie", "Taryn", "Corinne", "Rayna", "Asia", "Jaylin", "Noemi", "Carlee", "Abbigail", "Aryana", "Ayleen", "Eileen", "Livia", "Lillianna", "Mara", "Danika", "Mina", "Aliya", "Paloma", "Aimee", "Kaya", "Kora", "Tabitha", "Denise", "Hadassah", "Kayden", "Monroe", "Briley", "Celia", "Sandra", "Elaine", "Hana", "Jolie", "Kristina", "Myra", "Milana", "Lisa", "Renata", "Zariyah", "Adrienne", "America", "Emmalee", "Zaniyah", "Celine", "Cherish", "Jaida", "Kimora", "Mariyah", "Avah", "Nola", "Iliana", "Chana", "Cindy", "Janiya", "Carolyn", "Marisol", "Maliah", "Galilea", "Kiana", "Milania", "Alaya", "Bryn", "Emory", "Lorelai", "Jocelynn", "Yamileth", "Martha", "Jenny", "Keyla", "Alyvia", "Wren", "Dorothy", "Jordynn", "Amirah", "Nathaly", "Taliyah", "Zaria", "Deborah", "Elin", "Rylan", "Aubrianna", "Yasmin", "Julianne", "Zion", "Roselyn", "Salma", "Ivanna", "Joyce", "Paulina", "Lilith", "Saniyah", "Janae", "Aubrielle", "Ayanna", "Henley", "Sutton", "Aurelia", "Lesly", "Remi", "Britney", "Heather", "Barbara", "Bryleigh", "Emmalynn", "Kaitlynn", "Elliot", "Milena", "Susan", "Ariyah", "Kyndall", "Paula", "Thalia", "Aubri", "Kaleigh", "Tegan", "Yaritza", "Angeline", "Mercy", "Kairi", "Kourtney", "Krystal", "Carla", "Carter", "Mercedes", "Alannah", "Lina", "Sonia", "Kenia", "Everleigh", "Ivory", "Sloan", "Abril", "Alisha", "Katalina", "Carlie", "Lara", "Laurel", "Scarlette", "Carley", "Dixie", "Miya", "Micah", "Regan", "Samiyah", "Charlize", "Sharon", "Rosie", "Aviana", "Aleigha", "Gwyneth", "Sky", "Estella", "Hadlee", "Luz", "Patience", "Temperance", "Ingrid", "Raina", "Libby", "Jurnee", "Zahra", "Belen", "Jewel", "Anabel", "Marianna", "Renee", "Rory", "Elliott", "Karlie", "Saylor", "Deanna", "Freya", "Lilia", "Marjorie", "Sidney", "Tara", "Azaria", "Campbell", "Kai", "Ann", "Destinee", "Ariya", "Lilyanna", "Avianna", "Macey", "Shannon", "Lennon", "Saniya", "Haleigh", "Jolene", "Liv", "Oakley", "Esme", "Hunter", "Aliza", "Amalia", "Annalee", "Evalyn", "Giavanna", "Karis", "Kaylen", "Rayne", "Audriana", "Emerie", "Giada", "Harlee", "Kori", "Margot", "Abrielle", "Ellison", "Gwen", "Moriah", "Wynter", "Alisson", "Belinda", "Cristina", "Lillyana", "Neriah", "Rihanna", "Tamia", "Rivka", "Annabell", "Araceli", "Ayana", "Emmaline", "Giovanna", "Kylah", "Kailani", "Karissa", "Nahla", "Zainab", "Devyn", "Karma", "Marleigh", "Meadow", "India", "Kaiya", "Sarahi", "Audrianna", "Natalya", "Bayleigh", "Estelle", "Kaidence", "Kaylyn", "Magnolia", "Princess", "Avalyn", "Ireland", "Jayde", "Roxanne", "Alaysia", "Amia", "Astrid", "Karly", "Dalilah", "Makena", "Penny", "Ryann", "Charity", "Judith", "Kenna", "Tess", "Tinley", "Collins", "Noah", "Liam", "Jacob", "Mason", "William", "Ethan", "Michael", "Alexander", "Jayden", "Daniel", "Elijah", "Aiden", "James", "Benjamin", "Matthew", "Jackson", "Logan", "David", "Anthony", "Joseph", "Joshua", "Andrew", "Lucas", "Gabriel", "Samuel", "Christopher", "John", "Dylan", "Isaac", "Ryan", "Nathan", "Carter", "Caleb", "Luke", "Christian", "Hunter", "Henry", "Owen", "Landon", "Jack", "Wyatt", "Jonathan", "Eli", "Isaiah", "Sebastian", "Jaxon", "Julian", "Brayden", "Gavin", "Levi", "Aaron", "Oliver", "Jordan", "Nicholas", "Evan", "Connor", "Charles", "Jeremiah", "Cameron", "Adrian", "Thomas", "Robert", "Tyler", "Colton", "Austin", "Jace", "Angel", "Dominic", "Josiah", "Brandon", "Ayden", "Kevin", "Zachary", "Parker", "Blake", "Jose", "Chase", "Grayson", "Jason", "Ian", "Bentley", "Adam", "Xavier", "Cooper", "Justin", "Nolan", "Hudson", "Easton", "Jase", "Carson", "Nathaniel", "Jaxson", "Kayden", "Brody", "Lincoln", "Luis", "Tristan", "Damian", "Camden", "Juan", "Vincent", "Bryson", "Ryder", "Asher", "Carlos", "Jesus", "Micah", "Maxwell", "Mateo", "Alex", "Max", "Leo", "Elias", "Cole", "Miles", "Silas", "Bryce", "Eric", "Brantley", "Sawyer", "Declan", "Braxton", "Kaiden", "Colin", "Timothy", "Santiago", "Antonio", "Giovanni", "Hayden", "Diego", "Leonardo", "Bryan", "Miguel", "Roman", "Jonah", "Steven", "Ivan", "Kaleb", "Wesley", "Richard", "Jaden", "Victor", "Ezra", "Joel", "Edward", "Jayce", "Aidan", "Preston", "Greyson", "Brian", "Kaden", "Ashton", "Alan", "Patrick", "Kyle", "Riley", "George", "Jesse", "Jeremy", "Marcus", "Harrison", "Jude", "Weston", "Ryker", "Alejandro", "Jake", "Axel", "Grant", "Maddox", "Theodore", "Emmanuel", "Cayden", "Emmett", "Brady", "Bradley", "Gael", "Malachi", "Oscar", "Abel", "Tucker", "Jameson", "Caden", "Abraham", "Mark", "Sean", "Ezekiel", "Kenneth", "Gage", "Everett", "Kingston", "Nicolas", "Zayden", "King", "Bennett", "Calvin", "Avery", "Tanner", "Paul", "Kai", "Maximus", "Rylan", "Luca", "Graham", "Omar", "Derek", "Jayceon", "Jorge", "Peter", "Peyton", "Devin", "Collin", "Andres", "Jaiden", "Cody", "Zane", "Amir", "Corbin", "Francisco", "Xander", "Eduardo", "Conner", "Javier", "Jax", "Myles", "Griffin", "Iker", "Garrett", "Damien", "Simon", "Zander", "Seth", "Travis", "Charlie", "Cristian", "Trevor", "Zion", "Lorenzo", "Dean", "Gunner", "Chance", "Elliot", "Lukas", "Cash", "Elliott", "Israel", "Manuel", "Josue", "Jasper", "Keegan", "Finn", "Spencer", "Stephen", "Fernando", "Ricardo", "Mario", "Jeffrey", "Shane", "Clayton", "Reid", "Erick", "Cesar", "Paxton", "Martin", "Raymond", "Judah", "Trenton", "Johnny", "Andre", "Tyson", "Beau", "Landen", "Caiden", "Maverick", "Dominick", "Troy", "Kyler", "Hector", "Cruz", "Beckett", "Johnathan", "Donovan", "Edwin", "Kameron", "Marco", "Drake", "Edgar", "Holden", "Rafael", "Dante", "Jaylen", "Emiliano", "Waylon", "Andy", "Alexis", "Rowan", "Felix", "Drew", "Emilio", "Gregory", "Karter", "Brooks", "Dallas", "Lane", "Anderson", "Jared", "Skyler", "Angelo", "Shawn", "Aden", "Erik", "Dalton", "Fabian", "Sergio", "Milo", "Louis", "Titus", "Kendrick", "Braylon", "August", "Dawson", "Reed", "Emanuel", "Arthur", "Jett", "Leon", "Brendan", "Frank", "Marshall", "Emerson", "Desmond", "Derrick", "Colt", "Karson", "Messiah", "Zaiden", "Braden", "Amari", "Roberto", "Romeo", "Joaquin", "Malik", "Walter", "Brennan", "Pedro", "Knox", "Nehemiah", "Julius", "Grady", "Allen", "Ali", "Archer", "Kamden", "Dakota", "Maximiliano", "Ruben", "Quinn", "Barrett", "Tate", "Corey", "Adan", "Braylen", "Marcos", "Remington", "Phillip", "Kason", "Major", "Kellan", "Cohen", "Walker", "Gideon", "Taylor", "River", "Jayson", "Brycen", "Abram", "Cade", "Matteo", "Dillon", "Damon", "Dexter", "Kolton", "Phoenix", "Noel", "Brock", "Porter", "Philip", "Enrique", "Leland", "Ty", "Esteban", "Danny", "Jay", "Gerardo", "Keith", "Kellen", "Gunnar", "Armando", "Zachariah", "Orion", "Ismael", "Colby", "Pablo", "Ronald", "Atticus", "Trey", "Quentin", "Ryland", "Kash", "Raul", "Enzo", "Julio", "Darius", "Rodrigo", "Landyn", "Donald", "Bruce", "Jakob", "Kade", "Ari", "Keaton", "Albert", "Muhammad", "Rocco", "Solomon", "Rhett", "Cason", "Jaime", "Scott", "Chandler", "Mathew", "Maximilian", "Russell", "Dustin", "Ronan", "Tony", "Cyrus", "Jensen", "Hugo", "Saul", "Trent", "Deacon", "Davis", "Colten", "Malcolm", "Mohamed", "Devon", "Izaiah", "Randy", "Ibrahim", "Jerry", "Prince", "Tristen", "Alec", "Chris", "Dennis", "Clark", "Gustavo", "Mitchell", "Rory", "Jamison", "Leonel", "Finnegan", "Pierce", "Nash", "Kasen", "Khalil", "Darren", "Moses", "Issac", "Adriel", "Lawrence", "Braydon", "Jaxton", "Alberto", "Justice", "Curtis", "Larry", "Warren", "Zayne", "Yahir", "Jimmy", "Uriel", "Finley", "Nico", "Thiago", "Armani", "Jacoby", "Jonas", "Rhys", "Casey", "Tobias", "Frederick", "Jaxen", "Kobe", "Franklin", "Ricky", "Talon", "Ace", "Marvin", "Alonzo", "Arjun", "Jalen", "Alfredo", "Moises", "Sullivan", "Francis", "Case", "Brayan", "Alijah", "Arturo", "Lawson", "Raylan", "Mekhi", "Nikolas", "Carmelo", "Byron", "Nasir", "Reece", "Royce", "Sylas", "Ahmed", "Mauricio", "Beckham", "Roy", "Payton", "Raiden", "Korbin", "Maurice", "Ellis", "Aarav", "Johan", "Gianni", "Kayson", "Aldo", "Arian", "Isaias", "Jamari", "Kristopher", "Uriah", "Douglas", "Kane", "Milan", "Skylar", "Dorian", "Tatum", "Wade", "Cannon", "Quinton", "Bryant", "Toby", "Dane", "Sam", "Moshe", "Asa", "Mohammed", "Joe", "Kieran", "Roger", "Channing", "Daxton", "Ezequiel", "Orlando", "Matias", "Malakai", "Nathanael", "Zackary", "Boston", "Ahmad", "Dominik", "Lance", "Alvin", "Conor", "Odin", "Cullen", "Mohammad", "Deandre", "Benson", "Gary", "Blaine", "Carl", "Sterling", "Nelson", "Kian", "Salvador", "Luka", "Nikolai", "Nixon", "Niko", "Bowen", "Kyrie", "Brenden", "Callen", "Vihaan", "Luciano", "Terry", "Demetrius", "Raphael", "Ramon", "Xzavier", "Amare", "Rohan", "Reese", "Quincy", "Eddie", "Noe", "Yusuf", "London", "Hayes", "Jefferson", "Matthias", "Kelvin", "Terrance", "Madden", "Bentlee", "Layne", "Harvey", "Sincere", "Kristian", "Julien", "Melvin", "Harley", "Emmitt", "Neil", "Rodney", "Winston", "Hank", "Ayaan", "Ernesto", "Jeffery", "Alessandro", "Lucian", "Rex", "Wilson", "Mathias", "Memphis", "Princeton", "Santino", "Jon", "Tripp", "Lewis", "Trace", "Dax", "Eden", "Joey", "Nickolas", "Neymar", "Bruno", "Marc", "Crosby", "Cory", "Kendall", "Abdullah", "Allan", "Davion", "Hamza", "Soren", "Brentley", "Jasiah", "Edison", "Harper", "Tommy", "Morgan", "Zain", "Flynn", "Roland", "Theo", "Chad", "Lee", "Bobby", "Rayan", "Samson", "Brett", "Kylan", "Branson", "Bronson", "Ray", "Arlo", "Lennox", "Stanley", "Zechariah", "Kareem", "Micheal", "Reginald", "Alonso", "Casen", "Guillermo", "Leonard", "Augustus", "Tomas", "Billy", "Conrad", "Aryan", "Makai", "Elisha", "Westin", "Otto", "Adonis", "Jagger", "Keagan", "Dayton", "Leonidas", "Kyson", "Brodie", "Alden", "Aydin", "Valentino", "Harry", "Willie", "Yosef", "Braeden", "Marlon", "Terrence", "Lamar", "Shaun", "Aron", "Blaze", "Layton", "Duke", "Legend", "Jessie", "Terrell", "Clay", "Dwayne", "Felipe", "Kamari", "Gerald", "Kody", "Kole", "Maxim", "Omari", "Chaim", "Crew", "Lionel", "Vicente", "Bo", "Sage", "Rogelio", "Jermaine", "Gauge", "Will", "Emery", "Giovani", "Ronnie", "Elian", "Hendrix", "Javon", "Rayden", "Alexzander", "Ben", "Camron", "Jamarion", "Kolby", "Remy", "Jamal", "Urijah", "Jaydon", "Kyree", "Ariel", "Braiden", "Cassius", "Triston", "Jerome", "Junior", "Landry", "Wayne", "Killian", "Jamie", "Davian", "Lennon", "Samir", "Oakley", "Rene", "Ronin", "Tristian", "Darian", "Giancarlo", "Jadiel", "Amos", "Eugene", "Mayson", "Vincenzo", "Alfonso", "Brent", "Cain", "Callan", "Leandro", "Callum", "Darrell", "Atlas", "Fletcher", "Jairo", "Jonathon", "Kenny", "Tyrone", "Adrien", "Markus", "Thaddeus", "Zavier", "Marcel", "Marquis", "Misael", "Abdiel", "Draven", "Ishaan", "Lyric", "Ulises", "Jamir", "Marcelo", "Davin", "Bodhi", "Justus", "Mack", "Rudy", "Cedric", "Craig", "Frankie", "Javion", "Maxton", "Deshawn", "Jair", "Duncan", "Hassan", "Gibson", "Isiah", "Cayson", "Darwin", "Kale", "Kolten", "Lucca", "Kase", "Konner", "Konnor", "Randall", "Azariah", "Stefan", "Enoch", "Kymani", "Dominique", "Maximo", "Van", "Forrest", "Alvaro", "Gannon", "Jordyn", "Rolando", "Sonny", "Brice", "Coleman", "Yousef", "Aydan", "Ean", "Johnathon", "Quintin", "Semaj", "Cristopher", "Harlan", "Vaughn", "Zeke", "Axton", "Damion", "Jovanni", "Fisher", "Heath", "Ramiro", "Seamus", "Vance", "Yael", "Jadon", "Kamdyn", "Rashad", "Camdyn", "Jedidiah", "Santos", "Steve", "Chace", "Marley", "Brecken", "Kamryn", "Valentin", "Dilan", "Mike", "Krish", "Salvatore", "Brantlee", "Gilbert", "Turner", "Camren", "Franco", "Hezekiah", "Zaid", "Anders", "Deangelo", "Harold", "Joziah", "Mustafa", "Emory", "Jamar", "Reuben", "Royal", "Zayn", "Arnav", "Bently", "Gavyn", "Ares", "Ameer", "Juelz", "Rodolfo", "Titan", "Bridger", "Briggs", "Cortez", "Blaise", "Demarcus", "Rey", "Hugh", "Benton", "Giovanny", "Tristin", "Aidyn", "Jovani", "Jaylin", "Jorden", "Kaeden", "Clinton", "Efrain", "Kingsley", "Makhi", "Aditya", "Teagan", "Jericho", "Kamron", "Xavi", "Ernest", "Kaysen", "Zaire", "Deon", "Foster", "Lochlan", "Gilberto", "Gino", "Izayah", "Maison", "Miller", "Antoine", "Garrison", "Rylee", "Cristiano", "Dangelo", "Keenan", "Stetson", "Truman", "Brysen", "Jaycob", "Kohen", "Augustine", "Castiel", "Langston", "Magnus", "Osvaldo", "Reagan", "Sidney", "Tyree", "Yair", "Deegan", "Kalel", "Todd", "Alfred", "Anson", "Apollo", "Rowen", "Santana", "Ephraim", "Houston", "Jayse", "Leroy", "Pierre", "Tyrell", "Camryn", "Grey", "Yadiel", "Aaden", "Corban", "Denzel", "Jordy", "Kannon", "Branden", "Brendon", "Brenton", "Dario", "Jakobe", "Lachlan", "Thatcher", "Immanuel", "Camilo", "Davon", "Graeme", "Rocky", "Broderick", "Clyde", "Darien"
    };
    // Source: http://names.mongabay.com/data/1000.html
    private static final String[] lastNames = {
            "Smith", "Johnson", "Williams", "Brown", "Jones", "Miller", "Davis", "Garcia", "Rodriguez", "Wilson", "Martinez", "Anderson", "Taylor", "Thomas", "Hernandez", "Moore", "Martin", "Jackson", "Thompson", "White", "Lopez", "Lee", "Gonzalez", "Harris", "Clark", "Lewis", "Robinson", "Walker", "Perez", "Hall", "Young", "Allen", "Sanchez", "Wright", "King", "Scott", "Green", "Baker", "Adams", "Nelson", "Hill", "Ramirez", "Campbell", "Mitchell", "Roberts", "Carter", "Phillips", "Evans", "Turner", "Torres", "Parker", "Collins", "Edwards", "Stewart", "Flores", "Morris", "Nguyen", "Murphy", "Rivera", "Cook", "Rogers", "Morgan", "Peterson", "Cooper", "Reed", "Bailey", "Bell", "Gomez", "Kelly", "Howard", "Ward", "Cox", "Diaz", "Richardson", "Wood", "Watson", "Brooks", "Bennett", "Gray", "James", "Reyes", "Cruz", "Hughes", "Price", "Myers", "Long", "Foster", "Sanders", "Ross", "Morales", "Powell", "Sullivan", "Russell", "Ortiz", "Jenkins", "Gutierrez", "Perry", "Butler", "Barnes", "Fisher", "Henderson", "Coleman", "Simmons", "Patterson", "Jordan", "Reynolds", "Hamilton", "Graham", "Kim", "Gonzales", "Alexander", "Ramos", "Wallace", "Griffin", "West", "Cole", "Hayes", "Chavez", "Gibson", "Bryant", "Ellis", "Stevens", "Murray", "Ford", "Marshall", "Owens", "Mcdonald", "Harrison", "Ruiz", "Kennedy", "Wells", "Alvarez", "Woods", "Mendoza", "Castillo", "Olson", "Webb", "Washington", "Tucker", "Freeman", "Burns", "Henry", "Vasquez", "Snyder", "Simpson", "Crawford", "Jimenez", "Porter", "Mason", "Shaw", "Gordon", "Wagner", "Hunter", "Romero", "Hicks", "Dixon", "Hunt", "Palmer", "Robertson", "Black", "Holmes", "Stone", "Meyer", "Boyd", "Mills", "Warren", "Fox", "Rose", "Rice", "Moreno", "Schmidt", "Patel", "Ferguson", "Nichols", "Herrera", "Medina", "Ryan", "Fernandez", "Weaver", "Daniels", "Stephens", "Gardner", "Payne", "Kelley", "Dunn", "Pierce", "Arnold", "Tran", "Spencer", "Peters", "Hawkins", "Grant", "Hansen", "Castro", "Hoffman", "Hart", "Elliott", "Cunningham", "Knight", "Bradley", "Carroll", "Hudson", "Duncan", "Armstrong", "Berry", "Andrews", "Johnston", "Ray", "Lane", "Riley", "Carpenter", "Perkins", "Aguilar", "Silva", "Richards", "Willis", "Matthews", "Chapman", "Lawrence", "Garza", "Vargas", "Watkins", "Wheeler", "Larson", "Carlson", "Harper", "George", "Greene", "Burke", "Guzman", "Morrison", "Munoz", "Jacobs", "Obrien", "Lawson", "Franklin", "Lynch", "Bishop", "Carr", "Salazar", "Austin", "Mendez", "Gilbert", "Jensen", "Williamson", "Montgomery", "Harvey", "Oliver", "Howell", "Dean", "Hanson", "Weber", "Garrett", "Sims", "Burton", "Fuller", "Soto", "Mccoy", "Welch", "Chen", "Schultz", "Walters", "Reid", "Fields", "Walsh", "Little", "Fowler", "Bowman", "Davidson", "May", "Day", "Schneider", "Newman", "Brewer", "Lucas", "Holland", "Wong", "Banks", "Santos", "Curtis", "Pearson", "Delgado", "Valdez", "Pena", "Rios", "Douglas", "Sandoval", "Barrett", "Hopkins", "Keller", "Guerrero", "Stanley", "Bates", "Alvarado", "Beck", "Ortega", "Wade", "Estrada", "Contreras", "Barnett", "Caldwell", "Santiago", "Lambert", "Powers", "Chambers", "Nunez", "Craig", "Leonard", "Lowe", "Rhodes", "Byrd", "Gregory", "Shelton", "Frazier", "Becker", "Maldonado", "Fleming", "Vega", "Sutton", "Cohen", "Jennings", "Parks", "Mcdaniel", "Watts", "Barker", "Norris", "Vaughn", "Vazquez", "Holt", "Schwartz", "Steele", "Benson", "Neal", "Dominguez", "Horton", "Terry", "Wolfe", "Hale", "Lyons", "Graves", "Haynes", "Miles", "Park", "Warner", "Padilla", "Bush", "Thornton", "Mccarthy", "Mann", "Zimmerman", "Erickson", "Fletcher", "Mckinney", "Page", "Dawson", "Joseph", "Marquez", "Reeves", "Klein", "Espinoza", "Baldwin", "Moran", "Love", "Robbins", "Higgins", "Ball", "Cortez", "Le", "Griffith", "Bowen", "Sharp", "Cummings", "Ramsey", "Hardy", "Swanson", "Barber", "Acosta", "Luna", "Chandler", "Blair", "Daniel", "Cross", "Simon", "Dennis", "Oconnor", "Quinn", "Gross", "Navarro", "Moss", "Fitzgerald", "Doyle", "Mclaughlin", "Rojas", "Rodgers", "Stevenson", "Singh", "Yang", "Figueroa", "Harmon", "Newton", "Paul", "Manning", "Garner", "Mcgee", "Reese", "Francis", "Burgess", "Adkins", "Goodman", "Curry", "Brady", "Christensen", "Potter", "Walton", "Goodwin", "Mullins", "Molina", "Webster", "Fischer", "Campos", "Avila", "Sherman", "Todd", "Chang", "Blake", "Malone", "Wolf", "Hodges", "Juarez", "Gill", "Farmer", "Hines", "Gallagher", "Duran", "Hubbard", "Cannon", "Miranda", "Wang", "Saunders", "Tate", "Mack", "Hammond", "Carrillo", "Townsend", "Wise", "Ingram", "Barton", "Mejia", "Ayala", "Schroeder", "Hampton", "Rowe", "Parsons", "Frank", "Waters", "Strickland", "Osborne", "Maxwell", "Chan", "Deleon", "Norman", "Harrington", "Casey", "Patton", "Logan", "Bowers", "Mueller", "Glover", "Floyd", "Hartman", "Buchanan", "Cobb", "French", "Kramer", "Mccormick", "Clarke", "Tyler", "Gibbs", "Moody", "Conner", "Sparks", "Mcguire", "Leon", "Bauer", "Norton", "Pope", "Flynn", "Hogan", "Robles", "Salinas", "Yates", "Lindsey", "Lloyd", "Marsh", "Mcbride", "Owen", "Solis", "Pham", "Lang", "Pratt", "Lara", "Brock", "Ballard", "Trujillo", "Shaffer", "Drake", "Roman", "Aguirre", "Morton", "Stokes", "Lamb", "Pacheco", "Patrick", "Cochran", "Shepherd", "Cain", "Burnett", "Hess", "Li", "Cervantes", "Olsen", "Briggs", "Ochoa", "Cabrera", "Velasquez", "Montoya", "Roth", "Meyers", "Cardenas", "Fuentes", "Weiss", "Hoover", "Wilkins", "Nicholson", "Underwood", "Short", "Carson", "Morrow", "Colon", "Holloway", "Summers", "Bryan", "Petersen", "Mckenzie", "Serrano", "Wilcox", "Carey", "Clayton", "Poole", "Calderon", "Gallegos", "Greer", "Rivas", "Guerra", "Decker", "Collier", "Wall", "Whitaker", "Bass", "Flowers", "Davenport", "Conley", "Houston", "Huff", "Copeland", "Hood", "Monroe", "Massey", "Roberson", "Combs", "Franco", "Larsen", "Pittman", "Randall", "Skinner", "Wilkinson", "Kirby", "Cameron", "Bridges", "Anthony", "Richard", "Kirk", "Bruce", "Singleton", "Mathis", "Bradford", "Boone", "Abbott", "Charles", "Allison", "Sweeney", "Atkinson", "Horn", "Jefferson", "Rosales", "York", "Christian", "Phelps", "Farrell", "Castaneda", "Nash", "Dickerson", "Bond", "Wyatt", "Foley", "Chase", "Gates", "Vincent", "Mathews", "Hodge", "Garrison", "Trevino", "Villarreal", "Heath", "Dalton", "Valencia", "Callahan", "Hensley", "Atkins", "Huffman", "Roy", "Boyer", "Shields", "Lin", "Hancock", "Grimes", "Glenn", "Cline", "Delacruz", "Camacho", "Dillon", "Parrish", "Oneill", "Melton", "Booth", "Kane", "Berg", "Harrell", "Pitts", "Savage", "Wiggins", "Brennan", "Salas", "Marks", "Russo", "Sawyer", "Baxter", "Golden", "Hutchinson", "Liu", "Walter", "Mcdowell", "Wiley", "Rich", "Humphrey", "Johns", "Koch", "Suarez", "Hobbs", "Beard", "Gilmore", "Ibarra", "Keith", "Macias", "Khan", "Andrade", "Ware", "Stephenson", "Henson", "Wilkerson", "Dyer", "Mcclure", "Blackwell", "Mercado", "Tanner", "Eaton", "Clay", "Barron", "Beasley", "Oneal", "Preston", "Small", "Wu", "Zamora", "Macdonald", "Vance", "Snow", "Mcclain", "Stafford", "Orozco", "Barry", "English", "Shannon", "Kline", "Jacobson", "Woodard", "Huang", "Kemp", "Mosley", "Prince", "Merritt", "Hurst", "Villanueva", "Roach", "Nolan", "Lam", "Yoder", "Mccullough", "Lester", "Santana", "Valenzuela", "Winters", "Barrera", "Leach", "Orr", "Berger", "Mckee", "Strong", "Conway", "Stein", "Whitehead", "Bullock", "Escobar", "Knox", "Meadows", "Solomon", "Velez", "Odonnell", "Kerr", "Stout", "Blankenship", "Browning", "Kent", "Lozano", "Bartlett", "Pruitt", "Buck", "Barr", "Gaines", "Durham", "Gentry", "Mcintyre", "Sloan", "Melendez", "Rocha", "Herman", "Sexton", "Moon", "Hendricks", "Rangel", "Stark", "Lowery", "Hardin", "Hull", "Sellers", "Ellison", "Calhoun", "Gillespie", "Mora", "Knapp", "Mccall", "Morse", "Dorsey", "Weeks", "Nielsen", "Livingston", "Leblanc", "Mclean", "Bradshaw", "Glass", "Middleton", "Buckley", "Schaefer", "Frost", "Howe", "House", "Mcintosh", "Ho", "Pennington", "Reilly", "Hebert", "Mcfarland", "Hickman", "Noble", "Spears", "Conrad", "Arias", "Galvan", "Velazquez", "Huynh", "Frederick", "Randolph", "Cantu", "Fitzpatrick", "Mahoney", "Peck", "Villa", "Michael", "Donovan", "Mcconnell", "Walls", "Boyle", "Mayer", "Zuniga", "Giles", "Pineda", "Pace", "Hurley", "Mays", "Mcmillan", "Crosby", "Ayers", "Case", "Bentley", "Shepard", "Everett", "Pugh", "David", "Mcmahon", "Dunlap", "Bender", "Hahn", "Harding", "Acevedo", "Raymond", "Blackburn", "Duffy", "Landry", "Dougherty", "Bautista", "Shah", "Potts", "Arroyo", "Valentine", "Meza", "Gould", "Vaughan", "Fry", "Rush", "Avery", "Herring", "Dodson", "Clements", "Sampson", "Tapia", "Bean", "Lynn", "Crane", "Farley", "Cisneros", "Benton", "Ashley", "Mckay", "Finley", "Best", "Blevins", "Friedman", "Moses", "Sosa", "Blanchard", "Huber", "Frye", "Krueger", "Bernard", "Rosario", "Rubio", "Mullen", "Benjamin", "Haley", "Chung", "Moyer", "Choi", "Horne", "Yu", "Woodward", "Ali", "Nixon", "Hayden", "Rivers", "Estes", "Mccarty", "Richmond", "Stuart", "Maynard", "Brandt", "Oconnell", "Hanna", "Sanford", "Sheppard", "Church", "Burch", "Levy", "Rasmussen", "Coffey", "Ponce", "Faulkner", "Donaldson", "Schmitt", "Novak", "Costa", "Montes", "Booker", "Cordova", "Waller", "Arellano", "Maddox", "Mata", "Bonilla", "Stanton", "Compton", "Kaufman", "Dudley", "Mcpherson", "Beltran", "Dickson", "Mccann", "Villegas", "Proctor", "Hester", "Cantrell", "Daugherty", "Cherry", "Bray", "Davila", "Rowland", "Levine", "Madden", "Spence", "Good", "Irwin", "Werner", "Krause", "Petty", "Whitney", "Baird", "Hooper", "Pollard", "Zavala", "Jarvis", "Holden", "Haas", "Hendrix", "Mcgrath", "Bird", "Lucero", "Terrell", "Riggs", "Joyce", "Mercer", "Rollins", "Galloway", "Duke", "Odom", "Andersen", "Downs", "Hatfield", "Benitez", "Archer", "Huerta", "Travis", "Mcneil", "Hinton", "Zhang", "Hays", "Mayo", "Fritz", "Branch", "Mooney", "Ewing", "Ritter", "Esparza", "Frey", "Braun", "Gay", "Riddle", "Haney", "Kaiser", "Holder", "Chaney", "Mcknight", "Gamble", "Vang", "Cooley", "Carney", "Cowan", "Forbes", "Ferrell", "Davies", "Barajas", "Shea", "Osborn", "Bright", "Cuevas", "Bolton", "Murillo", "Lutz", "Duarte", "Kidd", "Key", "Cooke"
    };
    // @formatter:on

    /**
     * Get a random full name.
     *
     * @param rand The rand instance.
     * @return The full name.
     */
    public static String getFullName(Random rand)
    {
        final String ret = getFirstName(rand) + " " + getLastName(rand);
        Log.i(TAG, "getFullName:" + ret);
        return ret;
    }

    /**
     * Get a random first name.
     *
     * @param rand The rand instance.
     * @return The first name.
     */
    public static String getFirstName(Random rand)
    {
        return firstNames[rand.nextInt(firstNames.length)];
    }

    /**
     * Get a random last name.
     *
     * @param rand The rand instance.
     * @return The last name.
     */
    public static String getLastName(Random rand)
    {
        return lastNames[rand.nextInt(lastNames.length)];
    }
}