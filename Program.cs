internal class Program
{
    private static void Main(string[] args)
    {
        Console.WriteLine("I want to play a game~!!\n");

        Random random = new Random();

        int a1 = random.Next(0, 9);
        int a2 = random.Next(0, 9);
        int a3 = random.Next(0, 9);

        while (((a1 != a2) && (a2 != a3) && (a1 != a3)) == false)
        {
            if (a1 == a2)
            {
                a2 = random.Next(0, 9);
            }
            else if (a2 == a3)
            {
                a3 = random.Next(0, 9);
            }
            else if (a1 == a3)
            {
                a3 = random.Next(0, 9);
            }
        }

        Console.WriteLine($"정답 : {a1} , {a2} , {a3}");

        int[] answerDigits = new int[3] { a1, a2, a3 };
        int strike = 0;
        int ball = 0;

        do
        {
            Console.WriteLine("\n세 자리 숫자를 입력하세요: ");
            string? answer = Console.ReadLine();

            if (string.IsNullOrEmpty(answer) || answer.Length != 3)
            {
                Console.WriteLine("\n세 자리 숫자를 입력해야 합니다.");
                continue;
            }

            if (!int.TryParse(answer, out int userInput) || userInput < 0 || userInput > 999)
            {
                Console.WriteLine("\n유효한 세 자리 숫자를 입력하세요.");
                continue;
            }

            CompareDigits(answerDigits, userInput, out strike, out ball);
            Console.WriteLine($"\n{userInput} : {strike} Strike, {ball} Ball");

        } while (strike != 3);

        Console.WriteLine("\nStrike Out~!!! Game Over.");
    }

    private static void CompareDigits(int[] answerDigits, int userInput, out int strike, out int ball)
    {
        strike = 0;
        ball = 0;
        int[] userDigits = new int[3]
        {
            userInput / 100 % 10,
            userInput / 10 % 10,
            userInput % 10
        };
        for (int i = 0; i < 3; i++)
        {
            if (userDigits[i] == answerDigits[i])
            {
                strike++;
            }
            else if (Array.Exists(answerDigits, element => element == userDigits[i]))
            {
                ball++;
            }
        }
    }
}