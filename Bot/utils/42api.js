const axios = require("axios");
var intraAccessToken = undefined;

const apiRequest = (token) => {
  const instance = axios.create({
    baseURL: "https://api.intra.42.fr/v2/",
  });

  if (token) {
    instance.defaults.headers.common.Authorization = `Bearer ${token}`;
  } else {
    console.log("no token");
  }

  instance.interceptors.response.use(
    (response) => {
      return response;
    },
    function (error) {
      // Do something with response error
      return Promise.reject(error.response);
    }
  );

  return instance;
};

const getAccessAata = () => {
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
};

const generateNewToken = async () => {
  const now = new Date();
  intraAccessToken = await getAccessAata();
  intraAccessToken.expiresdate =
    now.getTime() + intraAccessToken.expires_in * 1000;
  console.info("generete new token ", intraAccessToken);
};

const fetchData = (login) => {
  return new Promise(async (resolve, reject) => {
    if (!intraAccessToken) {
      await generateNewToken();
    }
    if (new Date() >= new Date(intraAccessToken.expiresdate)) {
      console.info("The token has expired");
      await generateNewToken();
    } else {
      apiRequest(intraAccessToken.access_token)
        .get(`users/${login}`, {
          headers: {
            Authorization: `bearer ${intraAccessToken.access_token}`,
          },
        })
        .then(function (response) {
          resolve(response.data);
        })
        .catch(function (error) {
          reject(
            error.status == 404
              ? `Login '${login}' does not exist`
              : `42 API : ${error.data}`
          );
        });
    }
  });
};

module.exports = fetchData;
