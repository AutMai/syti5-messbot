using System.Collections.Concurrent;
using System.Globalization;
using System.IO.Ports;
using System.Text.RegularExpressions;

namespace View.Services;

public class ReadSerialPortService {
    private SerialPort mySerialPort;
    private MeasureResult SerialData { get; set; } = new();
    public readonly ConcurrentQueue<string> Commands = new();
    private object syncLock = new();

    public MeasureResult MeasureResult {
        get {
            lock (syncLock) {
                return SerialData;
            }
        }
        set {
            lock (syncLock) {
                SerialData = value;
            }
        }
    }

    public ReadSerialPortService() {
        mySerialPort = new SerialPort("COM3");

        mySerialPort.BaudRate = 9600;
        mySerialPort.NewLine = "]";


        new Thread(SerialPortPolling).Start();

        // start thread to handle commands
        var t = new Thread(HandleCommands);
        t.Start();
    }

    private void SerialPortPolling() {
        mySerialPort.Open();

        // poll in a loop if data is available
        while (true) {
            if (mySerialPort.BytesToRead > 0) {
                char nul = (char) 0;
                var data = mySerialPort.ReadLine().Replace(nul.ToString(), "");
                Console.WriteLine(data);
                Commands.Enqueue(data.Replace("[", ""));
            }
        }
    }

    public MeasureResult GetSerialValue() {
        return MeasureResult;
    }

    private void HandleCommands() {
        // always handle commands if there are any
        while (true) {
            if (Commands.Count > 0) {
                // read from concurrent queue with trydequeue
                Commands.TryDequeue(out var cmd);

                try {
                    MeasureResult measureResult = new();

                    if (cmd[0] == 6) {
                        Console.WriteLine("Received ACK");
                        continue;
                    }
                    if (cmd[0] == 21) {
                        Console.WriteLine("Received NACK");
                        continue;
                    }

                    var transmissionType = cmd.Split("|")[0];
                    var transmissionData = cmd.Split("|")[1];


                    if (transmissionType == "a") {
                        var light = transmissionData.Split(";")[0];
                        var switchStatus = transmissionData.Split(";")[1];

                        measureResult.Luminosity = double.Parse(light.Split(":")[1], CultureInfo.InvariantCulture);
                        measureResult.DigitalStatus = switchStatus.Split(":")[1];
                    }
                    else if (transmissionType == "l") {
                        measureResult.Luminosity = double.Parse(transmissionData.Split(":")[1], CultureInfo.InvariantCulture);
                    }
                    else if (transmissionType == "s") {
                        measureResult.DigitalStatus = transmissionData.Split(":")[1];
                    }

                    MeasureResult = measureResult;
                }
                catch (Exception e) {
                    Console.WriteLine("Error while handling command: " + cmd);
                }
            }
        }
    }

    private void Write(string cmd) {
        mySerialPort.Write($"[{cmd}]");
    }

    public List<string> GetCommands(string input) {
        List<string> strings = new List<string>();

        // Create a regular expression pattern to match strings between "[" and "]"
        string pattern = @"\[(.*?)\]";

        // Create a regular expression object
        Regex regex = new Regex(pattern);

        // Find all matches in the input string
        MatchCollection matches = regex.Matches(input);

        // Iterate over the matches and add the captured group value to the list of strings
        foreach (Match match in matches) {
            strings.Add(match.Groups[1].Value);
        }

        return strings;
    }

    public void ChangeTransmission(bool transmitSwitch, bool transmitLight) {
        // send command to arduino
        if (transmitLight && transmitSwitch) {
            Write("a");
        }
        else if (transmitLight) {
            Write("l");
        }
        else if (transmitSwitch) {
            Write("s");
        }
        else {
            Write("q");
        }
    }

    public void ChangeTransmissionInterval(string interval) {
        Write($"ti{interval}");
    }
    
    public void SingleTransmission() => Write("req_a");

    public void SingleSwitchTransmission() => Write($"req_s");

    public void SingleLightTransmission() => Write($"req_l");
}

public class MeasureResult {
    public double Luminosity { get; set; }
    public string DigitalStatus { get; set; }

    public override string ToString() {
        return $"Luminosity: {Luminosity}, DigitalStatus: {DigitalStatus}";
    }
}