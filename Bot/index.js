var irc = require("irc");
const utils = require("./utils");
const HOST = process.env.HOST ? process.env.HOST : "localhost"
const PASSWORD = process.env.PASSWORD
const PORT = process.env.PORT ? process.env.PORT : 6667

var client = new irc.Client(HOST, "bot", {
  password: PASSWORD,
  port: PORT,
  userName: "Bot",
  realName: "1337 Bot",
  encoding: "UTF-8",
  messageSplit:100,
  floodProtection: true,
  floodProtectionDelay: 1,
});

const channels = new Set([])

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

client.addListener("invite", function (channel, from, message) {
  client.join(channel, (data) => {
    client.say(
      channel,
      `Hello members of ${channel} :D ! Type "!help" to get available commands.`
    );
    console.log(`I have joined the channel ${channel}`);
  });

  if (!channels.has(channel)) {
    client.addListener(`kick${channel}`, function (nick, by, reason, message) {
      client.say(channel, `3lach jriti 3lia a si ${by} :(`);
    });

    client.addListener(`message${channel}`, async function (from, message) {
      const args = message.split(" ");

      if (commands[message]) {
        await commands[message](client, channel);
      } else if (commandsWithParams[args[0]]) {
        await commandsWithParams[args[0]](client, channel, args[1], args[0]);
      }
    });
  }
  channels.add(channel);
});

client.addListener("error", function(message) {
  console.log("Got error from server (" + message.rawCommand + "): " + message.command);
});
