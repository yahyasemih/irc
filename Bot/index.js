var irc = require("irc");
const utils = require("./utils");

var client = new irc.Client("localhost", "bot", {
  channels: ["#hello"],
  password: "lsaidi",
  port: 4242,
  // debug: true,
  userName: "Bot",
  realName: "Im a bot from 1337 of Bots ;)",
  // showErrors: true,
  // autoRejoin: true, // auto rejoin channel when kicked
  // autoConnect: true, // persistence to connect
  encoding: "UTF-8",
  messageSplit:100,
  floodProtection: true,
  floodProtectionDelay: 70,
});

// client.addListener('message', function (from, to, message) {
//     console.log('form server '+ from + ' => ' + to + ': ' + message);
// });

client.addListener("invite", function (channel, from, message) {
  // console.log(channel + ' => ' + from + ': ' + message);

  client.join(channel, (data) => {
    client.say(
      channel,
      `Hello members of ${channel} :D ! Type "!help" to get available commands.`
    );
    console.log(`i have joined the channel ${channel}`);
  });

  client.addListener(`kick${channel}`, function (nick, by, reason, message) {
    client.say(channel, `3lach jriti 3lia a si ${by} :(`);
  });

  client.addListener(`message${channel}`, async function (from, message) {
    console.log(from + " => #yourchannel: " + message);

    const args = message.split(" ");

    const commands = {
      "!part": utils.part_cmd,
      "!time": utils.time_cmd,
      "!help": utils.help_cmd,
    };

    const commandsWithParams = {
      "!meteo": utils.meteo_cmd,
      "!ft_is_available": utils.is_available_cmd,
      "!ft_points": utils.ft_points_cmd,
      "!ft_wallet": utils.ft_wallet_cmd,
      "!ft_cursus_info": utils.ft_cursus_info_cmd
    };

    if (commands[message]) {
      await commands[message](client, channel);
    } else if (commandsWithParams[args[0]]) {
      await commandsWithParams[args[0]](client, channel, args[1], args[0]);
    }
  });
});
