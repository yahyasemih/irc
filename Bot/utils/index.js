const axios = require("axios");
var intraAccessToken = undefined;

async function help_cmd(cl, ch) {
  const message = `
  Available commands :
      !part            : leave the current channel
      !time            : gives the current time
      !help            : display this message
      !meteo           : display the meteo of a city (usage !meteo 'city')
      !ft_is_available : display the user if it's avilable in the cluster (usage !ft_is_available 'login')
      !ft_points       : display the point for the user (usage !ft_points 'login')
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
  console.log(city);
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
      .catch(function (error) {
        console.log(error);
        cl.say(ch, "city or location not found");
      });
  }
}

async function is_available_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_is_available <login>");
  } else {
    if (!intraAccessToken) {
      await accessToken42().then(({ access_token }) => {
        intraAccessToken = access_token;
      });
    }
    axios
      .get(`https://api.intra.42.fr/v2/users/${login}`, {
        headers: {
          Authorization: `bearer ${intraAccessToken}`,
        },
      })
      .then(({ data }) => {
        const message = data.location
          ? "Available : " + data.location
          : "Unavailable";
        cl.say(ch, message);
      })
      .catch((err) => {
        console.log(err);
        const message = "Login (" + login + ") dose not exist";
        cl.say(ch, message);
      });
  }
}

async function ft_cursus_info_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_cursus_info <login>");
  } else {
    if (!intraAccessToken) {
      await accessToken42().then(({ access_token }) => {
        intraAccessToken = access_token;
      });
    }
    axios
      .get(`https://api.intra.42.fr/v2/users/${login}`, {
        headers: {
          Authorization: `bearer ${intraAccessToken}`,
        },
      })
      .then(({ data }) => {
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
        console.log(err);
        const message = "Login (" + login + ") dose not exist";
        cl.say(ch, message);
      });
  }
}

async function ft_points_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_points <login>");
  } else {
    if (!intraAccessToken) {
      await accessToken42().then(({ access_token }) => {
        intraAccessToken = access_token;
      });
    }

    console.log(intraAccessToken);
    axios
      .get(`https://api.intra.42.fr/v2/users/${login}`, {
        headers: {
          Authorization: `bearer ${intraAccessToken}`,
        },
      })
      .then(({ data }) => {
        const message = data.correction_point;
        cl.say(ch, message);
      })
      .catch((err) => {
        console.log(err);
        const message = "Login (" + login + ") dose not exist";
        cl.say(ch, message);
      });
  }
}

async function ft_wallet_cmd(cl, ch, login) {
  if (!login) {
    cl.say(ch, "usage !ft_wallet <login>");
  } else {
    if (!intraAccessToken) {
      await accessToken42().then(({ access_token }) => {
        intraAccessToken = access_token;
      });
    }
    axios
      .get(`https://api.intra.42.fr/v2/users/${login}`, {
        headers: {
          Authorization: `bearer ${intraAccessToken}`,
        },
      })
      .then(({ data }) => {
        const message =
          data.wallet > 0 ? data.wallet : "[" + login + "] have 0 (mrakal)";
        cl.say(ch, message);
      })
      .catch((err) => {
        console.log(err);
        const message = "Login (" + login + ") dose not exist";
        cl.say(ch, message);
      });
  }
}

function accessToken42() {
  return new Promise((resolve, reject) => {
    axios
      .request({
        url: "/oauth/token",
        method: "post",
        baseURL: "https://api.intra.42.fr/",
        auth: {
          username:
            "f08595cb84f22c093234355aaff97227f7331edad0f190d931bfbe2e7980e0ce",
          password:
            "60e994a064df5ef7eef313ac7e62260b54b4b885cfd6a2688ab069083cf84f12",
        },
        data: {
          grant_type: "client_credentials",
          scope: "public",
        },
      })
      .then(function (response) {
        resolve(response.data);
      })
      .catch(function (error) {
        reject(error);
      });
  });
}

module.exports = {
  accessToken42,
  part_cmd,
  time_cmd,
  help_cmd,
  meteo_cmd,
  is_available_cmd,
  ft_cursus_info_cmd,
  ft_points_cmd,
  ft_wallet_cmd,
};
