const axios = require("axios");
const fetchData = require("./42api");

async function help_cmd(cl, ch) {
  const message = `
  Available commands :
    !part            : leave the current channel
    !time            : give the current time
    !help            : display this message
    !meteo           : display the meteo of a city (usage !meteo 'city')
    !ft_is_available : display availability of the user (usage !ft_is_available 'login')
    !ft_points       : display the correction points of the user (usage !ft_points 'login')
    !ft_wallet       : display the wallet for the user (usage !ft_wallet 'login')
    !ft_cursus_info  : display the user levels infos (usage !ft_cursus_info 'login')`;
  await cl.say(ch, message);
}

function part_cmd(cl, ch) {
  cl.say(ch, `Goodbye members of ${ch} :'(`);
  cl.part(ch, "leaving ...");
}

function time_cmd(cl, ch) {
  let d = new Date();
  let format = d.toLocaleString([], { dateStyle: "full", timeStyle: "short" });
  cl.say(ch, format);
}

function meteo_cmd(cl, ch, city) {
  if (!city) {
    cl.say(ch, "usage !meteo <city>");
  } else {
    ///https://rapidapi.com/visual-crossing-corporation-visual-crossing-corporation-default/api/visual-crossing-weather/
    const options = {
      method: "GET",
      url: "https://visual-crossing-weather.p.rapidapi.com/forecast",
      params: { aggregateHours: "24", location: city, contentType: "json" },
      headers: {
        "X-RapidAPI-Host": "visual-crossing-weather.p.rapidapi.com",
        "X-RapidAPI-Key": "5dee8b835dmshf1b5efbf74673f2p14df90jsn8f15e5683985",
      },
    };

    axios
      .request(options)
      .then(function ({ data }) {
        const { locations } = data;
        const message = `Search city :   ${locations[city].name}
        Address: ${locations[city].address}
        Timezone: ${locations[city].tz}
        Weather by days: `;
        cl.say(ch, message);
        if (locations[city].values)
          for (let index = 0; index < locations[city].values.length; index++) {
            const element = locations[city].values[index];
            const message = `
            day : ${new Date(element.datetimeStr).toLocaleDateString("fr-MA", {
              year: "numeric",
              month: "2-digit",
              day: "2-digit",
            })}           conditions:    ${element.conditions}`;
            cl.say(ch, message);
          }
      })
      .catch(function (err) {
        cl.say(ch, "city or location not found");
      });
  }
}

async function is_available_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_is_available <login>");
  } else {
    fetchData(login)
      .then((data) => {
        const message = data.location
          ? `${login} is available : ${data.location}`
          : `${login} is unavailable`;
        cl.say(ch, message);
      })
      .catch((err) => {
        cl.say(ch, err);
      });
  }
}

async function ft_cursus_info_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_cursus_info <login>");
  } else {
    fetchData(login)
      .then((data) => {
        let message = "";
        if (data.cursus_users.length) {
          message = `[${login}] available cursus and gades:`;
          cl.say(ch, message);
        }

        data.cursus_users.map((r) => {
          const grade = r.grade ? r.grade : "-";
          const level = r.level.toString().includes(".")
            ? r.level.toFixed(1)
            : r.level;
          message = `
          Cursus id    :   ${r.cursus_id}
          Cursus name  :   ${r.cursus.name}
          Level        :   ${level}
          Grade        :   ${grade}
          ________________________`;
          cl.say(ch, message);
        });
      })
      .catch((err) => {
        cl.say(ch, err);
      });
  }
}

async function ft_points_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_points <login>");
  } else {
    fetchData(login)
      .then((data) => {
        const message = `${login} has ${data.correction_point} points`;
        cl.say(ch, message);
      })
      .catch((err) => {
        cl.say(ch, err);
      });
  }
}

async function ft_wallet_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_wallet <login>");
  } else {
    fetchData(login)
      .then((data) => {
        const message =
          data.wallet > 0
            ? `${login} has ${data.wallet} points`
            : "[" + login + "] have 0 (mrakal)";
        cl.say(ch, message);
      })
      .catch((err) => {
        cl.say(ch, err);
      });
  }
}

module.exports = {
  part_cmd,
  time_cmd,
  help_cmd,
  meteo_cmd,
  is_available_cmd,
  ft_cursus_info_cmd,
  ft_points_cmd,
  ft_wallet_cmd,
};
