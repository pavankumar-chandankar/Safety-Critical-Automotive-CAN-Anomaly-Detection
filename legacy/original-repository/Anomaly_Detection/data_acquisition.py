import pandas as pd
import io
import requests
import logging
import pandas as pd
import logging

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def load_public_transport_data(filepath="hourly_transport_demand.csv"):
    """
    Loads the public transport demand dataset.

    Args:
        filepath (str): The path to the dataset file.

    Returns:
        pd.DataFrame: The loaded DataFrame, or None if an error occurs.
    """
    try:
        logging.info(f"Loading data from: {filepath}")
        data = pd.read_csv(filepath, index_col="Datetime", parse_dates=True)
        logging.info("Data loaded successfully.")
        return data
    except FileNotFoundError:
        logging.error(f"Error: Dataset file not found at {filepath}")
        return None
    except Exception as e:
        logging.error(f"Error loading data: {e}")
        return None

if __name__ == "__main__":
    transport_data = load_public_transport_data()
    if transport_data is not None:
        logging.info("Data acquisition completed.")
        logging.info(f"Shape of the DataFrame: {transport_data.shape}")
        logging.info(f"Column names: {transport_data.columns.tolist()}")
        logging.info("First 5 rows:")
        logging.info(transport_data.head())
        logging.info("Data types:")
        logging.info(transport_data.info())
    else:
        logging.error("Data acquisition failed.")

# Configure logging
logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')

def download_data(url: str, names: list) -> pd.DataFrame:
    """
    Downloads data from a URL and loads it into a Pandas DataFrame.

    Args:
        url (str): The URL of the data file.
        names (list): A list of column names for the DataFrame.

    Returns:
        pd.DataFrame: The loaded DataFrame, or None if an error occurs.
    """
    try:
        logging.info(f"Downloading data from: {url}")
        response = requests.get(url)
        response.raise_for_status()  # Raise HTTPError for bad responses (4xx or 5xx)
        data = pd.read_csv(io.StringIO(response.content.decode('utf-8')), names=names)
        logging.info("Data downloaded and loaded successfully.")
        return data
    except requests.exceptions.RequestException as e:
        logging.error(f"Error downloading data: {e}")
        return None
    except pd.errors.ParserError as e:
        logging.error(f"Error parsing CSV data: {e}")
        return None

def inspect_data(df: pd.DataFrame) -> None:
    """
    Performs initial inspection of the DataFrame.

    Args:
        df (pd.DataFrame): The DataFrame to inspect.
    """
    if df is not None:
        logging.info("Inspecting data...")
        logging.info(f"Shape of the DataFrame: {df.shape}")
        logging.info(f"Column names: {df.columns.tolist()}")
        logging.info("First 5 rows:")
        logging.info(df.head())
        logging.info("Data types:")
        logging.info(df.dtypes)
        logging.info("Summary statistics:")
        logging.info(df.describe(include='all'))
        logging.info("Checking for missing values:")
        logging.info(df.isnull().sum())
    else:
        logging.warning("No DataFrame to inspect.")

def load_car_evaluation_data() -> pd.DataFrame:
    """
    Loads the Car Evaluation dataset.

    Returns:
        pd.DataFrame: The loaded DataFrame, or None if an error occurs.
    """
    url = "https://archive.ics.uci.edu/ml/machine-learning-databases/car/car.data"
    names = ['buying', 'maint', 'doors', 'persons', 'lug_boot', 'safety', 'class']
    df = download_data(url, names)
    inspect_data(df)
    return df

if __name__ == "__main__":
    car_data = load_car_evaluation_data()
    if car_data is not None:
        logging.info("Data acquisition completed.")
    else:
        logging.error("Data acquisition failed.")
