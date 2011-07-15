This archive contains three different versions of Fairy.

Fairy Light contains the basic alpha-beta search, move generation procedures, and a very stupid evaluation.  The only search improvement in this version is the hashtable, as that seemed too basic to leave out.

Fairy Medium adds several search improvements (remembering PV-moves, using killer moves and hash moves, using nega-scout and aspiration windows), but still uses the sme stupid evaluation as Fairy Light.

Fairy Full doesn't add much to the search (only extensions, and they are not that vital), but includes the full evaluation as it currently looks in Fairy.  Fairy Full also includes my utility (runtest) to run a test on a series of positions, along with my current set of test positions.

Each of these folders contain all the .c and .h files needed, along with a project-file for Dev-C++, and an executable compiled on my computer.  I have not tried to compile with any other compiler than the MinGW distribution of gcc that is inclued with Dev-C++.

I wouldn't call the source code well-commented, but there are quite a few comments in there.  Further, I tend to use long variable and procedure names which should help with readability.  Still, if you have any questions, feel free to ask them in the Arimaa forum, and I'll do my best to answer.

I hope this archive is useful for somebody and inspires them to start or work more on their own Arimaa-bot.

Best regards
/Ola Hansson (unic)