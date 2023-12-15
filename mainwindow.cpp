void MainWindow::voteForIdeas()
{
    logTextEdit->append("Voting in progress...");
    QMap<int, QString> votes;
    for (int i = 0; i < ideas.size(); ++i)
    {
        int numVotes = rand() % 10; // Generate a random number between 0 and 9
        votes.insert(numVotes, ideas[i]);
    }
    QStringList winners;
    QMapIterator<int, QString> it(votes);
    it.toBack();
    while (winners.size() < 3 && it.hasPrevious())
    {
        it.previous();
        winners.append(it.value());
    }

    logTextEdit->append("Voting completed. Winners:");
    for (const QString &winner : winners)
    {
        logTextEdit->append(winner);
    }
    logTextEdit->append("Saving winners to a file...");
    QFile file("winners.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        for (const QString &winner : winners)
        {
            stream << winner << Qt::endl;
        }
        file.close();
        logTextEdit->append("Winners saved to winners.txt");
    }
    else
    {
        logTextEdit->append("Error: Unable to open winners.txt for writing.");
    }
    saveBoard();
    // Close the board
    closeBoard();
}
void MainWindow::closeBoard()
{
    if (clientSocket)
    {
        clientSocket->close();
        clientSocket = nullptr;
    }
    server->close();
    startButton->setEnabled(true);
    logTextEdit->append("Board closed.");
}
void MainWindow::saveBoard()
{
    logTextEdit->append("Saving board...");
    closeBoard();
}

// Input Output Function 

readIdeasFromFile("ideas.txt");
voteForIdeas();
writeIdeasToFile("ideas.txt");
saveBoard();

void MainWindow::readIdeasFromFile(const QString &fileName)
{
    logTextEdit->append("Reading ideas from file...");

    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        while (!stream.atEnd())
        {
            QString idea = stream.readLine();
            ideas.append(idea);
        }
        file.close();

        logTextEdit->append("Ideas read successfully.");
    }
    else
    {
        logTextEdit->append("Error: Unable to open the file for reading.");
    }
}

void MainWindow::writeIdeasToFile(const QString &fileName)
{
    logTextEdit->append("Writing ideas to file...");

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&file);
        for (const QString &idea : ideas)
        {
            stream << idea << Qt::endl;
        }
        file.close();

        logTextEdit->append("Ideas written to file successfully.");
    }
    else
    {
        logTextEdit->append("Error: Unable to open the file for writing.");
    }
}

// Input Output Function  END

